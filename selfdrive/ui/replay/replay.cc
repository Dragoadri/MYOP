#include "selfdrive/ui/replay/replay.h"

#include "cereal/services.h"
#include "selfdrive/camerad/cameras/camera_common.h"
#include "selfdrive/common/timing.h"
#include "selfdrive/hardware/hw.h"

int getch(void) {
  int ch;
  struct termios oldt;
  struct termios newt;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}

Replay::Replay(QString route, SubMaster *sm_, QObject *parent) : sm(sm_), QObject(parent) {
  QStringList block = QString(getenv("BLOCK")).split(",");
  qDebug() << "blocklist" << block;

  QStringList allow = QString(getenv("ALLOW")).split(",");
  qDebug() << "allowlist" << allow;

  if (sm == nullptr) {
    ctx = Context::create();
    for (const auto &it : services) {
      std::string name = it.name;
      if ((allow[0].size() > 0 && !allow.contains(name.c_str())) ||
          block.contains(name.c_str())) {
        continue;
      }

      PubSocket *sock = PubSocket::create(ctx, name);
      if (sock == NULL) {
        qDebug() << "FAILED " << name.c_str();
        continue;
      }
      socks.insert(name, sock);
    }
  }

  current_segment = -window_padding - 1;

  bool create_jwt = true;
#if !defined(QCOM) && !defined(QCOM2)
  create_jwt = false;
#endif
  http = new HttpRequest(this, "https://api.commadotai.com/v1/route/" + route + "/files", "", create_jwt);
  QObject::connect(http, &HttpRequest::receivedResponse, this, &Replay::parseResponse);
}

void Replay::parseResponse(const QString &response){
  QJsonDocument doc = QJsonDocument::fromJson(response.trimmed().toUtf8());
  if (doc.isNull()) {
    qDebug() << "JSON Parse failed";
    return;
  }

  camera_paths = doc["cameras"].toArray();
  log_paths = doc["logs"].toArray();

  seekTime(0);
}

void Replay::addSegment(int i){
  assert((i >= 0) && (i < log_paths.size()) && (i < camera_paths.size()));
  if (lrs.find(i) != lrs.end()) {
    return;
  }

  QThread* lr_thread = new QThread;
  lrs.insert(i, new LogReader(log_paths.at(i).toString(), &events, &events_lock, &eidx));

  lrs[i]->moveToThread(lr_thread);
  QObject::connect(lr_thread, &QThread::started, lrs[i], &LogReader::process);
  lr_thread->start();

  frs.insert(i, new FrameReader(qPrintable(camera_paths.at(i).toString())));
}

void Replay::trimSegment(int n){
  if (lrs.contains(n)) {
    auto lr = lrs.take(n);
    delete lr;
  }
  if (frs.contains(n)) {
    auto fr = frs.take(n);
    delete fr;
  }

  events_lock.lockForWrite();
  auto eit = events.begin();
  while (eit != events.end()) {
    if(std::abs(eit.key()/1e9 - getCurrentTime()/1e9) > window_padding*60.0){
      eit = events.erase(eit);
      continue;
    }
    eit++;
  }
  events_lock.unlock();
}

void Replay::start(){
  thread = new QThread;
  QObject::connect(thread, &QThread::started, [=](){
    stream();
  });
  thread->start();

  kb_thread = new QThread;
  QObject::connect(kb_thread, &QThread::started, [=](){
    keyboardThread();
  });
  kb_thread->start();

  queue_thread = new QThread;
  QObject::connect(queue_thread, &QThread::started, [=](){
    segmentQueueThread();
  });
  queue_thread->start();
}

void Replay::seekTime(int ts){
  qInfo() << "seeking to " << ts;
  current_segment = ts/60;
  addSegment(current_segment);
}

void Replay::segmentQueueThread() {
  while (true) {
    // TODO: maintain the segment window
    QThread::sleep(1);
  }
}

void Replay::keyboardThread() {
  char c;
  while (true) {
    c = getch();
    if(c == '\n'){
      printf("Enter seek request: ");
      std::string r;
      std::cin >> r;

      if(r[0] == '#') {
        r.erase(0, 1);
        seekTime(std::stoi(r)*60);
      } else {
        seekTime(std::stoi(r));
      }
      getch(); // remove \n from entering seek
    } else if (c == 'm') {
      //seek_queue.enqueue({true, 60});
    } else if (c == 'M') {
      //seek_queue.enqueue({true, -60});
    } else if (c == 's') {
      //seek_queue.enqueue({true, 10});
    } else if (c == 'S') {
      //seek_queue.enqueue({true, -10});
    } else if (c == 'G') {
      //seek_queue.clear();
      //seek_queue.enqueue({false, 0});
    }
  }
}

void Replay::stream() {
  QElapsedTimer timer;
  timer.start();

  while (true) {
    if (events.size() == 0) {
      qDebug() << "waiting for events";
      QThread::msleep(100);
      continue;
    }

    uint64_t t0 = (events.begin()+1).key();
    uint64_t t0r = timer.nsecsElapsed();
    qDebug() << "unlogging at" << t0;

    // wait for future events to be ready?
    auto eit = events.lowerBound(t0);
    while(eit.key() - t0 > 1e9){
      eit = events.lowerBound(t0);
    }

    while ((eit != events.end())) {
      // what does this do?
      /*
      if (seeking) {
        t0 = route_start_ts + 1;
        tc = t0;
        qDebug() << "seeking to" << t0;
        t0r = timer.nsecsElapsed();
        eit = events.lowerBound(t0);
        if ((eit == events.end()) || (eit.key() - t0 > 1e9)) {
          qWarning() << "seek off end";
          while((eit == events.end()) || (eit.key() - t0 > 1e9)) {
            qDebug() << "stuck";
            eit = events.lowerBound(t0);
            QThread::sleep(1);
            printf("%f\n", (eit.key() - t0)/1e9);
          }
        }
        seeking = false;
      }
      */

      cereal::Event::Reader e = (*eit);
      std::string type;
      KJ_IF_MAYBE(e_, static_cast<capnp::DynamicStruct::Reader>(e).which()) {
        type = e_->getProto().getName();
      }

      if (type == "initData") {
        route_start_ts = e.getLogMonoTime();
      }

      uint64_t tm = e.getLogMonoTime();
      auto it = socks.find(type);
      tc = tm;
      if (it != socks.end()) {
        long etime = tm-t0;

        float timestamp = (tm - route_start_ts)/1e9;
        if(std::abs(timestamp-last_print) > 5.0){
          last_print = timestamp;
          printf("at %f\n", last_print);
        }

        long rtime = timer.nsecsElapsed() - t0r;
        long us_behind = ((etime-rtime)*1e-3)+0.5;
        if (us_behind > 0) {
          if (us_behind > 1e6) {
            qWarning() << "OVER ONE SECOND BEHIND, HACKING" << us_behind;
            us_behind = 0;
            t0 = tm;
            t0r = timer.nsecsElapsed();
          }
          QThread::usleep(us_behind);
          //qDebug() << "sleeping" << us_behind << etime << timer.nsecsElapsed();
        }

        // publish frame
        // TODO: publish all frames
        if (type == "roadCameraState") {
          auto fr = e.getRoadCameraState();

          auto it_ = eidx.find(fr.getFrameId());
          if (it_ != eidx.end()) {
            auto pp = *it_;
            if (frs.find(pp.first) != frs.end()) {
              auto frm = frs[pp.first];
              auto data = frm->get(pp.second);

              if (vipc_server == nullptr) {
                cl_device_id device_id = cl_get_device_id(CL_DEVICE_TYPE_DEFAULT);
                cl_context context = CL_CHECK_ERR(clCreateContext(NULL, 1, &device_id, NULL, NULL, &err));

                vipc_server = new VisionIpcServer("camerad", device_id, context);
                vipc_server->create_buffers(VisionStreamType::VISION_STREAM_RGB_BACK, UI_BUF_COUNT, true, frm->width, frm->height);

                vipc_server->start_listener();
              }

              VisionBuf *buf = vipc_server->get_buffer(VisionStreamType::VISION_STREAM_RGB_BACK);
              memcpy(buf->addr, data, frm->getRGBSize());
              VisionIpcBufExtra extra = {};

              vipc_server->send(buf, &extra, false);
            }
          }
        }

        // publish msg
        if (sm == nullptr){
          capnp::MallocMessageBuilder msg;
          msg.setRoot(e);
          auto words = capnp::messageToFlatArray(msg);
          auto bytes = words.asBytes();

          (*it)->send((char*)bytes.begin(), bytes.size());
        } else{
          std::vector<std::pair<std::string, cereal::Event::Reader>> messages;
          messages.push_back({type, e});
          sm->update_msgs(nanos_since_boot(), messages);
        }
      }

      ++eit;
    }
  }
}
