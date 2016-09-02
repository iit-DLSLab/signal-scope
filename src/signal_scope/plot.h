#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt/qwt_plot.h>
#include <qwt/qwt_interval.h>
#include <qwt/qwt_system_clock.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_zoomer.h>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotGrid;
class QwtPlotDirectPainter;

class SignalData;
class MyMagnifier;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);
    virtual ~Plot();


    enum AlignMode
    {
      RIGHT,
      CENTER
    };


    void start();
    void stop();

    virtual void replot();

    void addSignal(SignalData* signalData, QColor color);
    void removeSignal(SignalData* signalData);
    void setSignalVisible(SignalData* signalData, bool visible);
    void setSignalColor(SignalData* signalData, QColor color);

    double timeWindow();

    void setEndTime(double endTime);
    void moveCanvas(int dx, int dy, QPoint pos);

    void setBackgroundColor(QString color);

    void setMarkerEnabled(bool enabled);
    void setAlignMode(AlignMode mode);

    bool isStopped();

    void flagAxisSyncRequired();

    void setPointSize(double pointSize);
    void setCurveStyle(QwtPlotCurve::CurveStyle style);

    double getExtent();
    void setExtent(double extent);

private slots:
    void onMovedMyCanvas(int, int, QPoint);

signals:
    void movedMyCanvasSignal(int, int, QPoint);
    void anotherCanvasMovedSignal(int, int, QPoint);
    void syncXAxisScale(double x0, double x1);

public Q_SLOTS:
    void setTimeWindow(double);
    void setYScale(double);

protected:

    void updateTicks();

private:
    void initBackground();
    QwtPlotZoomer* zoomer;
    QwtPlotMarker *d_origin;
    QwtPlotMarker *d_marker;
    QwtPlotGrid *d_grid;
    MyMagnifier *mMagnifier;
    bool mStopped;
    bool mAxisSyncRequired;
    int mColorMode;
    double mTimeWindow;
    AlignMode mAlignMode;

    QMap<SignalData*, QwtPlotCurve*> mSignals;
};

class MyPanner : public QObject
{
    Q_OBJECT
public:
    MyPanner(Plot* plot);
    QPoint mInitialPos;
    bool mEnabled;
    int mMouseButton;
    int mKeyboardButton;
    Plot* mPlot;
    bool eventFilter( QObject *object, QEvent *event );
    void moveCanvas( int dx, int dy );
    void widgetMousePressEvent( QMouseEvent *mouseEvent );
    void widgetMouseMoveEvent( QMouseEvent *mouseEvent );
    void widgetMouseReleaseEvent( QMouseEvent *mouseEvent );

signals:
    void movedCanvas(int, int, QPoint);

public slots:
    void moveCanvas( int dx, int dy, QPoint pos );
};

#endif
