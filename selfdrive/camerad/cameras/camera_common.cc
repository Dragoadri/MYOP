#include "selfdrive/camerad/cameras/camera_common.h"

#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <chrono>
#include <thread>

#include "libyuv.h"
#include <jpeglib.h>

#include "selfdrive/camerad/imgproc/utils.h"
#include "selfdrive/common/clutil.h"
#include "selfdrive/common/modeldata.h"
#include "selfdrive/common/params.h"
#include "selfdrive/common/swaglog.h"
#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"

#ifdef QCOM
#include "selfdrive/camerad/cameras/camera_qcom.h"
#elif QCOM2
#include "selfdrive/camerad/cameras/camera_qcom2.h"
#elif WEBCAM
#include "selfdrive/camerad/cameras/camera_webcam.h"
#else
#include "selfdrive/camerad/cameras/camera_frame_stream.h"
#endif

const int YUV_COUNT = 100;

static cl_program build_debayer_program(cl_device_id device_id, cl_context context, const CameraInfo *ci, const CameraBuf *b, const CameraState *s) {
  char args[4096];
  snprintf(args, sizeof(args),
           "-cl-fast-relaxed-math -cl-denorms-are-zero "
           "-DFRAME_WIDTH=%d -DFRAME_HEIGHT=%d -DFRAME_STRIDE=%d "
           "-DRGB_WIDTH=%d -DRGB_HEIGHT=%d -DRGB_STRIDE=%d "
           "-DBAYER_FLIP=%d -DHDR=%d -DCAM_NUM=%d",
           ci->frame_width, ci->frame_height, ci->frame_stride,
           b->rgb_width, b->rgb_height, b->rgb_stride,
           ci->bayer_flip, ci->hdr, s->camera_num);
  const char *cl_file = Hardware::TICI() ? "cameras/real_debayer.cl" : "cameras/debayer.cl";
  return cl_program_from_file(context, device_id, cl_file, args);
}

void CameraBuf::init(cl_device_id device_id, cl_context context, CameraState *s, VisionIpcServer * v, int frame_cnt, VisionStreamType rgb_type, VisionStreamType yuv_type, release_cb release_callback) {
  vipc_server = v;
  this->rgb_type = rgb_type;
  this->yuv_type = yuv_type;
  this->release_callback = release_callback;

  const CameraInfo *ci = &s->ci;
  camera_state = s;
  frame_buf_count = frame_cnt;

  // RAW frame
  const int frame_size = ci->frame_height * ci->frame_stride;
  camera_bufs = std::make_unique<VisionBuf[]>(frame_buf_count);
  camera_bufs_metadata = std::make_unique<FrameMetadata[]>(frame_buf_count);

  for (int i = 0; i < frame_buf_count; i++) {
    camera_bufs[i].allocate(frame_size);
    camera_bufs[i].init_cl(device_id, context);
  }

  rgb_width = ci->frame_width;
  rgb_height = ci->frame_height;

  if (!Hardware::TICI() && ci->bayer) {
    // debayering does a 2x downscale
    rgb_width = ci->frame_width / 2;
    rgb_height = ci->frame_height / 2;
  }

  yuv_transform = get_model_yuv_transform(ci->bayer);

  vipc_server->create_buffers(rgb_type, UI_BUF_COUNT, true, rgb_width, rgb_height);
  rgb_stride = vipc_server->get_buffer(rgb_type)->stride;

  vipc_server->create_buffers(yuv_type, YUV_COUNT, false, rgb_width, rgb_height);

  if (ci->bayer) {
    cl_program prg_debayer = build_debayer_program(device_id, context, ci, this, s);
    krnl_debayer = CL_CHECK_ERR(clCreateKernel(prg_debayer, "debayer10", &err));
    CL_CHECK(clReleaseProgram(prg_debayer));
  }

  rgb2yuv = std::make_unique<Rgb2Yuv>(context, device_id, rgb_width, rgb_height, rgb_stride);

#ifdef __APPLE__
  q = CL_CHECK_ERR(clCreateCommandQueue(context, device_id, 0, &err));
#else
  const cl_queue_properties props[] = {0};  //CL_QUEUE_PRIORITY_KHR, CL_QUEUE_PRIORITY_HIGH_KHR, 0};
  q = CL_CHECK_ERR(clCreateCommandQueueWithProperties(context, device_id, props, &err));
#endif
}

CameraBuf::~CameraBuf() {
  for (int i = 0; i < frame_buf_count; i++) {
    camera_bufs[i].free();
  }

  if (krnl_debayer) CL_CHECK(clReleaseKernel(krnl_debayer));
  if (q) CL_CHECK(clReleaseCommandQueue(q));
}

bool CameraBuf::acquire() {
  if (!safe_queue.try_pop(cur_buf_idx, 1)) return false;

  if (camera_bufs_metadata[cur_buf_idx].frame_id == -1) {
    LOGE("no frame data? wtf");
    release();
    return false;
  }

  cur_frame_data = camera_bufs_metadata[cur_buf_idx];
  cur_rgb_buf = vipc_server->get_buffer(rgb_type);

  cl_event debayer_event;
  cl_mem camrabuf_cl = camera_bufs[cur_buf_idx].buf_cl;
  if (camera_state->ci.bayer) {
    CL_CHECK(clSetKernelArg(krnl_debayer, 0, sizeof(cl_mem), &camrabuf_cl));
    CL_CHECK(clSetKernelArg(krnl_debayer, 1, sizeof(cl_mem), &cur_rgb_buf->buf_cl));
#ifdef QCOM2
    constexpr int localMemSize = (DEBAYER_LOCAL_WORKSIZE + 2 * (3 / 2)) * (DEBAYER_LOCAL_WORKSIZE + 2 * (3 / 2)) * sizeof(short int);
    const size_t globalWorkSize[] = {size_t(camera_state->ci.frame_width), size_t(camera_state->ci.frame_height)};
    const size_t localWorkSize[] = {DEBAYER_LOCAL_WORKSIZE, DEBAYER_LOCAL_WORKSIZE};
    CL_CHECK(clSetKernelArg(krnl_debayer, 2, localMemSize, 0));
    CL_CHECK(clEnqueueNDRangeKernel(q, krnl_debayer, 2, NULL, globalWorkSize, localWorkSize,
                                    0, 0, &debayer_event));
#else
    float digital_gain = camera_state->digital_gain;
    if ((int)digital_gain == 0) {
      digital_gain = 1.0;
    }
    CL_CHECK(clSetKernelArg(krnl_debayer, 2, sizeof(float), &digital_gain));
    const size_t debayer_work_size = rgb_height;  // doesn't divide evenly, is this okay?
    CL_CHECK(clEnqueueNDRangeKernel(q, krnl_debayer, 1, NULL,
                                    &debayer_work_size, NULL, 0, 0, &debayer_event));
#endif
  } else {
    assert(rgb_stride == camera_state->ci.frame_stride);
    CL_CHECK(clEnqueueCopyBuffer(q, camrabuf_cl, cur_rgb_buf->buf_cl, 0, 0,
                               cur_rgb_buf->len, 0, 0, &debayer_event));
  }

  clWaitForEvents(1, &debayer_event);
  CL_CHECK(clReleaseEvent(debayer_event));

  cur_yuv_buf = vipc_server->get_buffer(yuv_type);
  rgb2yuv->queue(q, cur_rgb_buf->buf_cl, cur_yuv_buf->buf_cl);

  VisionIpcBufExtra extra = {
                        cur_frame_data.frame_id,
                        cur_frame_data.timestamp_sof,
                        cur_frame_data.timestamp_eof,
  };
  vipc_server->send(cur_rgb_buf, &extra);
  vipc_server->send(cur_yuv_buf, &extra);

  return true;
}

void CameraBuf::release() {
  if (release_callback) {
    release_callback((void*)camera_state, cur_buf_idx);
  }
}

void CameraBuf::queue(size_t buf_idx) {
  safe_queue.push(buf_idx);
}

// common functions

void fill_frame_data(cereal::FrameData::Builder &framed, const FrameMetadata &frame_data) {
  framed.setFrameId(frame_data.frame_id);
  framed.setTimestampEof(frame_data.timestamp_eof);
  framed.setTimestampSof(frame_data.timestamp_sof);
  framed.setFrameLength(frame_data.frame_length);
  framed.setIntegLines(frame_data.integ_lines);
  framed.setGain(frame_data.gain);
  framed.setHighConversionGain(frame_data.high_conversion_gain);
  framed.setMeasuredGreyFraction(frame_data.measured_grey_fraction);
  framed.setTargetGreyFraction(frame_data.target_grey_fraction);
  framed.setLensPos(frame_data.lens_pos);
  framed.setLensSag(frame_data.lens_sag);
  framed.setLensErr(frame_data.lens_err);
  framed.setLensTruePos(frame_data.lens_true_pos);
}

kj::Array<uint8_t> get_frame_image(const CameraBuf *b) {
  static const int x_min = util::getenv("XMIN", 0);
  static const int y_min = util::getenv("YMIN", 0);
  static const int env_xmax = util::getenv("XMAX", -1);
  static const int env_ymax = util::getenv("YMAX", -1);
  static const int scale = util::getenv("SCALE", 1);

  assert(b->cur_rgb_buf);

  const int x_max = env_xmax != -1 ? env_xmax : b->rgb_width - 1;
  const int y_max = env_ymax != -1 ? env_ymax : b->rgb_height - 1;
  const int new_width = (x_max - x_min + 1) / scale;
  const int new_height = (y_max - y_min + 1) / scale;
  const uint8_t *dat = (const uint8_t *)b->cur_rgb_buf->addr;

  kj::Array<uint8_t> frame_image = kj::heapArray<uint8_t>(new_width*new_height*3);
  uint8_t *resized_dat = frame_image.begin();
  int goff = x_min*3 + y_min*b->rgb_stride;
  for (int r=0;r<new_height;r++) {
    for (int c=0;c<new_width;c++) {
      memcpy(&resized_dat[(r*new_width+c)*3], &dat[goff+r*b->rgb_stride*scale+c*3*scale], 3*sizeof(uint8_t));
    }
  }
  return kj::mv(frame_image);
}

static std::pair<uint8_t *, size_t> yuv420_to_jpeg(const CameraBuf *b, int thumbnail_width, int thumbnail_height) {
  std::unique_ptr<uint8[]> buf(new uint8_t[(thumbnail_width * thumbnail_height * 3) / 2]);
  uint8_t *y_plane = buf.get();
  uint8_t *u_plane = y_plane + thumbnail_width * thumbnail_height;
  uint8_t *v_plane = u_plane + (thumbnail_width * thumbnail_height) / 4;
  {
    int result = libyuv::I420Scale(
        b->cur_yuv_buf->y, b->rgb_width, b->cur_yuv_buf->u, b->rgb_width / 2, b->cur_yuv_buf->v, b->rgb_width / 2,
        b->rgb_width, b->rgb_height,
        y_plane, thumbnail_width, u_plane, thumbnail_width / 2, v_plane, thumbnail_width / 2,
        thumbnail_width, thumbnail_height, libyuv::kFilterNone);
    if (result != 0) {
      LOGE("Generate YUV thumbnail failed.");
      return {nullptr, 0};
    }
  }

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  uint8_t *thumbnail_buffer = nullptr;
  size_t thumbnail_len = 0;
  jpeg_mem_dest(&cinfo, &thumbnail_buffer, &thumbnail_len);

  cinfo.image_width = thumbnail_width;
  cinfo.image_height = thumbnail_height;
  cinfo.input_components = 3;

  jpeg_set_defaults(&cinfo);
  jpeg_set_colorspace(&cinfo, JCS_YCbCr);
  // configure sampling factors for yuv420.
  cinfo.comp_info[0].h_samp_factor = 2;  // Y
  cinfo.comp_info[0].v_samp_factor = 2;
  cinfo.comp_info[1].h_samp_factor = 1;  // U
  cinfo.comp_info[1].v_samp_factor = 1;
  cinfo.comp_info[2].h_samp_factor = 1;  // V
  cinfo.comp_info[2].v_samp_factor = 1;
  cinfo.raw_data_in = TRUE;

  jpeg_set_quality(&cinfo, 50, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW y[16], u[8], v[8];
  JSAMPARRAY planes[3]{y, u, v};

  for (int line = 0; line < cinfo.image_height; line += 16) {
    for (int i = 0; i < 16; ++i) {
      y[i] = y_plane + (line + i) * cinfo.image_width;
      if (i % 2 == 0) {
        int offset = (cinfo.image_width / 2) * ((i + line) / 2);
        u[i / 2] = u_plane + offset;
        v[i / 2] = v_plane + offset;
      }
    }
    jpeg_write_raw_data(&cinfo, planes, 16);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  return {thumbnail_buffer, thumbnail_len};
}

static void publish_thumbnail(PubMaster *pm, const CameraBuf *b) {
  auto [thumbnail, len] = yuv420_to_jpeg(b, b->rgb_width / 4, b->rgb_height / 4);
  if (thumbnail == nullptr) return;

  MessageBuilder msg;
  auto thumbnaild = msg.initEvent().initThumbnail();
  thumbnaild.setFrameId(b->cur_frame_data.frame_id);
  thumbnaild.setTimestampEof(b->cur_frame_data.timestamp_eof);
  thumbnaild.setThumbnail(kj::arrayPtr(thumbnail, len));

  pm->send("thumbnail", msg);
  free(thumbnail);
}

float set_exposure_target(const CameraBuf *b, int x_start, int x_end, int x_skip, int y_start, int y_end, int y_skip) {
  int lum_med;
  uint32_t lum_binning[256] = {0};
  const uint8_t *pix_ptr = b->cur_yuv_buf->y;

  unsigned int lum_total = 0;
  for (int y = y_start; y < y_end; y += y_skip) {
    for (int x = x_start; x < x_end; x += x_skip) {
      uint8_t lum = pix_ptr[(y * b->rgb_width) + x];
      lum_binning[lum]++;
      lum_total += 1;
    }
  }


  // Find mean lumimance value
  unsigned int lum_cur = 0;
  for (lum_med = 255; lum_med >= 0; lum_med--) {
    lum_cur += lum_binning[lum_med];

    if (lum_cur >= lum_total / 2) {
      break;
    }
  }

  return lum_med / 256.0;
}

extern ExitHandler do_exit;

void *processing_thread(MultiCameraState *cameras, CameraState *cs, process_thread_cb callback) {
  const char *thread_name = nullptr;
  if (cs == &cameras->road_cam) {
    thread_name = "RoadCamera";
  } else if (cs == &cameras->driver_cam) {
    thread_name = "DriverCamera";
  } else {
    thread_name = "WideRoadCamera";
  }
  set_thread_name(thread_name);

  uint32_t cnt = 0;
  while (!do_exit) {
    if (!cs->buf.acquire()) continue;

    callback(cameras, cs, cnt);

    if (cs == &(cameras->road_cam) && cameras->pm && cnt % 100 == 3) {
      // this takes 10ms???
      publish_thumbnail(cameras->pm, &(cs->buf));
    }
    cs->buf.release();
    ++cnt;
  }
  return NULL;
}

std::thread start_process_thread(MultiCameraState *cameras, CameraState *cs, process_thread_cb callback) {
  return std::thread(processing_thread, cameras, cs, callback);
}

static void driver_cam_auto_exposure(CameraState *c, SubMaster &sm) {
  static const bool is_rhd = Params().getBool("IsRHD");
  struct ExpRect {int x1, x2, x_skip, y1, y2, y_skip;};
  const CameraBuf *b = &c->buf;

  int x_offset = 0, y_offset = 0;
  int frame_width = b->rgb_width, frame_height = b->rgb_height;


  ExpRect def_rect;
  if (Hardware::TICI()) {
    x_offset = 630, y_offset = 156;
    frame_width = 668, frame_height = frame_width / 1.33;
    def_rect = {96, 1832, 2, 242, 1148, 4};
  } else {
    def_rect = {is_rhd ? 0 : b->rgb_width * 3 / 5, is_rhd ? b->rgb_width * 2 / 5 : b->rgb_width, 2,
                b->rgb_height / 3, b->rgb_height, 1};
  }

  static ExpRect rect = def_rect;
  // use driver face crop for AE
  if (Hardware::EON() && sm.updated("driverState")) {
    if (auto state = sm["driverState"].getDriverState(); state.getFaceProb() > 0.4) {
      auto face_position = state.getFacePosition();
      int x = is_rhd ? 0 : frame_width - (0.5 * frame_height);
      x += (face_position[0] * (is_rhd ? -1.0 : 1.0) + 0.5) * (0.5 * frame_height) + x_offset;
      int y = (face_position[1] + 0.5) * frame_height + y_offset;
      rect = {std::max(0, x - 72), std::min(b->rgb_width - 1, x + 72), 2,
              std::max(0, y - 72), std::min(b->rgb_height - 1, y + 72), 1};
    }
  }

  camera_autoexposure(c, set_exposure_target(b, rect.x1, rect.x2, rect.x_skip, rect.y1, rect.y2, rect.y_skip));
}

void common_process_driver_camera(SubMaster *sm, PubMaster *pm, CameraState *c, int cnt) {
  int j = Hardware::TICI() ? 1 : 3;
  if (cnt % j == 0) {
    sm->update(0);
    driver_cam_auto_exposure(c, *sm);
  }
  MessageBuilder msg;
  auto framed = msg.initEvent().initDriverCameraState();
  framed.setFrameType(cereal::FrameData::FrameType::FRONT);
  fill_frame_data(framed, c->buf.cur_frame_data);
  if (env_send_driver) {
    framed.setImage(get_frame_image(&c->buf));
  }
  pm->send("driverCameraState", msg);
}
