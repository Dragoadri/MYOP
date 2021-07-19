#pragma once

#include "selfdrive/common/util.h"
#include "selfdrive/ui/qt/api.h"
#include "selfdrive/ui/ui.h"

class RequestRepeater : public QObject {
  Q_OBJECT

public:
  HttpRequest *request(const QString &requestURL, const QString &cacheKey = "", int period = 0, bool while_onroad = false);

public slots:
  void offroadTransition(bool offroad);
  void displayPowerChanged(bool on);

private:
  RequestRepeater(QObject *parent = 0);
  void updateRequests();

  struct Request {
    bool while_onroad;
    QString url;
    QTimer *timer;
    HttpRequest *req;
    QString prevResp;
  };
  QVector<Request *> requests_;
  Params params_;
  bool offroad_ = false, awake_ = true;

  friend RequestRepeater *requestRepeater();
};

RequestRepeater *requestRepeater();
