#pragma once

#include <assert.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>

#include <memory>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "selfdrive/common/mat.h"
#include "selfdrive/modeld/transforms/loadyuv.h"
#include "selfdrive/modeld/transforms/transform.h"

constexpr int MODEL_WIDTH = 512;
constexpr int MODEL_HEIGHT = 256;
constexpr int MODEL_FRAME_SIZE = MODEL_WIDTH * MODEL_HEIGHT * 3 / 2;

const bool send_raw_pred = getenv("SEND_RAW_PRED") != NULL;

void softmax(const float* input, float* output, size_t len);
float softplus(float input);
float sigmoid(float input);

class ModelFrame {
 public:
  ModelFrame(cl_device_id device_id, cl_context context);
  ~ModelFrame();
  float* prepare(cl_mem yuv_cl, int width, int height, const mat3& transform, cl_mem *output);

  const int buf_size = MODEL_FRAME_SIZE * 2;

 private:
  Transform transform;
  LoadYUVState loadyuv;
  cl_command_queue q;
  cl_mem y_cl, u_cl, v_cl, net_input_cl;
  std::unique_ptr<float[]> input_frames;
};
