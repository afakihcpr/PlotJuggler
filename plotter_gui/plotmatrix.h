#ifndef _PLOT_MATRIX_H_
#define _PLOT_MATRIX_H_

#include <qframe.h>
#include <QGridLayout>
#include "plotdata.h"
#include "plotwidget.h"

class PlotMatrix: public QFrame
{
    Q_OBJECT

public:
    PlotMatrix(QString name, PlotDataMap *datamap,QWidget * parent = NULL );
    virtual ~PlotMatrix();

    void addRow();
    void addColumn();
    void removeColumn(int column_to_delete);
    void removeRow(int row_to_delete);

    void removeEmpty();

    int rowsCount() const;
    int colsCount() const;
    int plotCount() const;

    bool isRowEmpty(int row );
    bool isColumnEmpty(int row );

    PlotWidget* plotAt( int row, int column );
    const PlotWidget* plotAt( int row, int column ) const;

    PlotWidget* plotAt( int index );
    const PlotWidget* plotAt( int index ) const;

    void setAxisScale(int axisId, int row, int col,
                      double min, double max, double step = 0 );

    QDomElement xmlSaveState(QDomDocument &doc);

    bool xmlLoadState(QDomElement &plotmatrix_element );

    void updateLayout();

    void replot();

    void setHorizontalLink(bool linked);

    void setName(const QString &new_name) ;

    const QString& name() const;

    QGridLayout* gridLayout();

public slots:
    void maximumZoomOutHorizontal();

    void maximumZoomOutVertical();

    void maximumZoomOut();


private slots:
    //  void swapWidgetByName(QString name_a, QString name_b);
    void on_singlePlotScaleChanged(PlotWidget* modified_plot, QRectF range);

private:
    void alignAxes( int rowOrColumn, int axis );
    void alignScaleBorder( int rowOrColumn, int axis );
    PlotWidget *addPlotWidget( int row, int col);
    void swapPlots( int rowA, int colA, int rowB, int colB);

    QGridLayout *layout;
    int num_rows;
    int num_cols;
    bool _horizontal_link;

    PlotDataMap *_mapped_data;


    QString _name;

signals:
    void plotAdded(PlotWidget*);
    void undoableChange();

};

#endif
