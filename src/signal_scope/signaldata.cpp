#include "signaldata.h"

#include <qvector.h>
#include <qmutex.h>
#include <qreadwritelock.h>
#include <QDebug>
#include <limits>

#include "fpscounter.h"


class SignalData::PrivateData
{
public:
  PrivateData()
  {
    messageError = false;
    this->lastSampleTime = -std::numeric_limits<float>::max();
  }

  bool messageError;

  QReadWriteLock lock;

  QRectF boundingRect;

  QMutex mutex; // protecting pending values
  QVector<double> xvalues;
  QVector<double> yvalues;
  QVector<double> pendingxvalues;
  QVector<double> pendingyvalues;

  QVector<double> intervalxvalues;
  QVector<double> intervalyvalues;

  double lastSampleTime;

  FPSCounter fpsCounter;

};

namespace {
  int signalHistoryLength = 60*5;
};


SignalData::SignalData()
{
  d_data = new PrivateData();
}

SignalData::~SignalData()
{
  delete d_data;
}

int SignalData::getHistoryLength()
{
  return signalHistoryLength;
}

void SignalData::setHistoryLength(int historyLength)
{
  signalHistoryLength = historyLength;
}

int SignalData::size() const
{
  return d_data->intervalxvalues.size();
}

QPointF SignalData::value(int index) const
{
  return QPointF(d_data->intervalxvalues[index], d_data->intervalyvalues[index]);
}

double SignalData::lastSampleTime() const
{
  return d_data->lastSampleTime;
}

QRectF SignalData::boundingRect() const
{
  return d_data->boundingRect;
}

void SignalData::lock()
{
  d_data->mutex.lock();
}

void SignalData::unlock()
{
  d_data->mutex.unlock();
}

void SignalData::appendSample(double x, double y)
{
  d_data->mutex.lock();
  d_data->pendingxvalues.append(x);
  d_data->pendingyvalues.append(y);
  d_data->fpsCounter.update();
  d_data->mutex.unlock();
}

void SignalData::clear()
{
  d_data->mutex.lock();
  d_data->xvalues.clear();
  d_data->yvalues.clear();
  d_data->intervalxvalues.clear();
  d_data->intervalyvalues.clear();
  d_data->pendingxvalues.clear();
  d_data->pendingyvalues.clear();
  d_data->boundingRect = QRectF();
  d_data->lastSampleTime = -std::numeric_limits<float>::max();
  d_data->mutex.unlock();
}

void SignalData::flagMessageError()
{
  d_data->messageError = true;
}

bool SignalData::hasMessageError() const
{
  return d_data->messageError;
}

double SignalData::messageFrequency() const
{
  d_data->mutex.lock();
  double freq = d_data->fpsCounter.averageFPS();
  d_data->mutex.unlock();
  return freq;
}

void SignalData::updateInterval(double minTime, double maxTime)
{
  //printf("update interval: %.3f,  %.3f\n", minTime, maxTime);

  d_data->intervalxvalues.clear();
  d_data->intervalyvalues.clear();

  const size_t nvalues = d_data->xvalues.size();
  for (int i = 0; i < nvalues; ++i)
  {
    double sampleX = d_data->xvalues[i];

    if (sampleX >= minTime && sampleX <= maxTime)
    {
      d_data->intervalxvalues.append(sampleX);
      d_data->intervalyvalues.append(d_data->yvalues[i]);
    }
  }
}

QRectF SignalData::computeBounds()
{
  QMutexLocker locker(&d_data->mutex);


  QVector<double>& xvalues = d_data->xvalues;
  QVector<double>& yvalues = d_data->yvalues;
  const size_t n = xvalues.size();

  if (n == 0)
  {
    return QRectF();
  }

  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  double maxX = -std::numeric_limits<double>::max();
  double maxY = -std::numeric_limits<double>::max();

  for (size_t i = 0; i < n; ++i)
  {
    double x = xvalues[i];
    double y = yvalues[i];

    if (x < minX) minX = x;
    if (x > maxX) maxX = x;
    if (y < minY) minY = y;
    if (y > maxY) maxY = y;
  }
  
  return QRectF(minX, minY, maxX-minX, maxY-minY);
}

void SignalData::updateValues()
{
  QVector<double>& xvalues = d_data->xvalues;
  QVector<double>& yvalues = d_data->yvalues;

  d_data->mutex.lock();
  xvalues += d_data->pendingxvalues;
  yvalues += d_data->pendingyvalues;
  d_data->pendingxvalues.clear();
  d_data->pendingyvalues.clear();
  d_data->mutex.unlock();

  if (!xvalues.size())
  {
    return;
  }

  // All values that are older than five minutes will expire
  double expireTime = xvalues.back() - signalHistoryLength;

  int idx = 0;
  while (idx < xvalues.size() && xvalues[idx] < expireTime)
  {
    ++idx;
  }

  // if itr == begin(), then no values will be erased
  xvalues.erase(xvalues.begin(), xvalues.begin()+idx);
  yvalues.erase(yvalues.begin(), yvalues.begin()+idx);

  d_data->lastSampleTime = xvalues.back();
}
