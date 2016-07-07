#ifndef DragableWidget_H
#define DragableWidget_H

#include <memory>
#include <map>
#include <QObject>
#include <QTextEdit>
#include <QDomDocument>
#include <deque>
#include <QMessageBox>
#include <QTime>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "plotdata.h"

class PlotWidget: public QtCharts::QChartView
{
    Q_OBJECT

public:
    PlotWidget(PlotDataMap* datamap, QWidget *parent=0);
    virtual ~PlotWidget();

    bool addCurve(const QString&  name, bool do_replot = true);

    bool isEmpty();

    QDomElement xmlSaveState(QDomDocument &doc);

    bool xmlLoadState(QDomElement &element, QMessageBox::StandardButton* answer);

    QRectF currentBoundingRect();

    std::pair<float,float> maximumRangeX();

    std::pair<float,float> maximumRangeY(bool current_canvas = false);

//    CurveTracker* tracker();

    void setScale( QRectF rect, bool emit_signal = true );

    void activateLegent(bool activate);

    typedef struct {
        std::shared_ptr<QtCharts::QLineSeries> series;
        PlotDataPtr data;

    } PlotCurve;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) ;
    virtual void dragMoveEvent(QDragMoveEvent *event) ;
    virtual void dropEvent(QDropEvent *event) ;

    virtual void mousePressEvent(QMouseEvent *event) ;
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
    void swapWidgetsRequested(PlotWidget* source, PlotWidget* destination);
    void rectChanged(PlotWidget* self, QRectF rect );
    void undoableChange();
    void trackerMoved(QPointF pos);

public slots:

    void replot() ;

    void detachAllCurves();

    void zoomOut();

    void on_zoomOutHorizontal_triggered();

    void on_zoomOutVertical_triggered();

    void removeCurve(const QString& name);

private slots:
    void launchRemoveCurveDialog();
    void canvasContextMenuTriggered(const QPoint &pos);
    void on_changeColor_triggered();
    void on_showPoints_triggered(bool checked);
    void on_externallyResized(QRectF new_rect);
    void replotImpl() ;
private:


    std::map<QString, PlotCurve > _curve_list;

    QAction *_action_removeCurve;
    QAction *_action_removeAllCurves;
    QAction *_action_changeColors;
    QAction *_action_showPoints;
    QAction *_action_zoomOutHorizontally;
    QAction *_action_zoomOutVertically;


//    QwtPlotZoomer* _zoomer;
 //   PlotMagnifier* _magnifier;
 //   QwtPlotPanner* _panner;
    // QRectF _prev_bounding;
 //   CurveTracker* _tracker;
 //   QwtPlotLegendItem* _legend;

    void setAxisScale( int axisId, double min, double max, double step = 0 );

    PlotDataMap* _mapped_data;

    void buildActions();
    void buildLegend();

    int   _fps_counter;
    QTime _fps_timeStamp;

    QTimer _replot_timer;


};

#endif
