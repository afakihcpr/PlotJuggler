#ifndef PLOTDATA_QWT_H
#define PLOTDATA_QWT_H

#include <QColor>

#include "plotdata.h"



class PlotDataQwt: public QwtSeriesData<QPointF>
{
public:

   // PlotDataQwt();


    PlotDataQwt(PlotDataPtr base);

    virtual ~PlotDataQwt() {}

    virtual QPointF sample( size_t i ) const;
    virtual QRectF boundingRect() const;

    QRectF maximumBoundingRect(double min_X, double max_X);

    PlotDataPtr plot() { return _plot_data; }

    virtual size_t size() const;

   // void setRangeX(double x_left, double x_right);

    QColor colorHint() const;
    void setColorHint(QColor color);

    void setSubsampleFactor();

private:
    PlotDataPtr _plot_data;
    int _preferedColor;
    int _subsample;
};



#endif // PLOTDATA_H
