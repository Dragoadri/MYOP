#pragma once

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <bzlib.h>
#include <kj/array.h>
#include <capnp/serialize.h>

#if defined(QCOM) || defined(QCOM2)
const std::string LOG_ROOT = "/data/media/0/realdata";
#else
const std::string LOG_ROOT = util::getenv_default("HOME", "/.comma/media/0/realdata", "/data/media/0/realdata");
#endif

#define LOGGER_MAX_HANDLES 16

class BZFile {
 public:
  BZFile(const char *path);
  ~BZFile();
  void write(void* data, size_t size);
  inline void write(kj::ArrayPtr<capnp::byte> array) { write(array.begin(), array.size()); }

 private:
  FILE* file = nullptr;
  BZFILE* bz_file = nullptr;
};

typedef struct LoggerHandle {
  pthread_mutex_t lock;
  int refcnt;
  char segment_path[4096];
  char log_path[4096];
  char qlog_path[4096];
  char lock_path[4096];
  std::unique_ptr<BZFile> log, q_log;
} LoggerHandle;

typedef struct LoggerState {
  pthread_mutex_t lock;
  int part;
  kj::Array<capnp::word> init_data;
  char route_name[64];
  char log_name[64];
  bool has_qlog;

  LoggerHandle handles[LOGGER_MAX_HANDLES];
  LoggerHandle* cur_handle;
} LoggerState;

int logger_mkpath(char* file_path);
kj::Array<capnp::word> logger_build_boot();
kj::Array<capnp::word> logger_build_init_data();
void logger_init(LoggerState *s, const char* log_name, bool has_qlog);
int logger_next(LoggerState *s, const char* root_path,
                            char* out_segment_path, size_t out_segment_path_len,
                            int* out_part);
LoggerHandle* logger_get_handle(LoggerState *s);
void logger_close(LoggerState *s);
void logger_log(LoggerState *s, uint8_t* data, size_t data_size, bool in_qlog);

void lh_log(LoggerHandle* h, uint8_t* data, size_t data_size, bool in_qlog);
void lh_close(LoggerHandle* h);
