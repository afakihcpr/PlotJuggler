#include <qlayout.h>
#include <qpen.h>
#include "plotmatrix.h"


static int widget_uid = 0;

PlotMatrix::PlotMatrix(QString name, PlotDataMap *datamap, QWidget *parent ):
    QFrame( parent ),
    _mapped_data(datamap),
    _name(name)
{
    num_rows = 0;
    num_cols = 0;

    layout = new QGridLayout( this );

    _horizontal_link = true;
    updateLayout();
}


PlotWidget* PlotMatrix::addPlotWidget(int row, int col)
{
    PlotWidget *plot = new PlotWidget( _mapped_data, this );

    plot->setWindowTitle(QString("PlotWidget ") + QString::number(widget_uid++));

    connect( plot, SIGNAL(rectChanged(PlotWidget*,QRectF)), this, SLOT(on_singlePlotScaleChanged(PlotWidget*,QRectF)));

    plot->setAttribute(Qt::WA_DeleteOnClose);

    layout->addWidget( plot, row, col );

    emit plotAdded(plot);

    return plot;
}

void PlotMatrix::addRow()
{
    if( num_rows==0 && num_cols==0 )
    {
        addPlotWidget( 0, 0 );
        num_rows = 1;
        num_cols = 1;
    }
    else{
        for ( int col = 0; col < colsCount(); col++ )
        {
            addPlotWidget( num_rows, col );
        }
        num_rows++;
    }

    updateLayout();
}

void PlotMatrix::addColumn()
{
    if( num_rows==0 && num_cols==0 )
    {
        addPlotWidget( 0, 0 );
        num_rows = 1;
        num_cols = 1;
    }
    else {
        for ( int row = 0; row < rowsCount(); row++ )
        {
            addPlotWidget( row, num_cols );
        }
        num_cols++;
    }
    updateLayout();
}

void PlotMatrix::swapPlots( int rowA, int colA, int rowB, int colB)
{
    QWidget *widgetA = layout->itemAtPosition(rowA, colA)->widget();
    QWidget *widgetB = layout->itemAtPosition(rowB, colB)->widget();

    layout->removeItem( layout->itemAtPosition(rowA, colA) );
    layout->removeItem( layout->itemAtPosition(rowB, colB) );

    layout->addWidget(widgetA, rowB, colB);
    layout->addWidget(widgetB, rowA, colA);
}

void PlotMatrix::removeColumn(int column_to_delete)
{
    if(num_rows==1 && num_cols ==1 ) {
        return;
    }

    for(int col = column_to_delete; col< num_cols-1; col++)
    {
        for(int row=0; row< num_rows; row++)
        {
            this->swapPlots( row, col, row, col+1);
        }
    }
    for(int row=0; row< num_rows; row++)
    {
        plotAt( row, num_cols -1)->close();
    }
    num_cols--;
    if( num_cols == 0){
        num_rows = 0;
    }

    updateLayout();

}

void PlotMatrix::removeRow(int row_to_delete)
{
    if(num_rows==1 && num_cols ==1 ) {
        return;
    }

    for(int row = row_to_delete; row< num_rows-1; row++)
    {
        for(int col = 0; col< num_cols; col++)
        {
            this->swapPlots( row, col, row+1, col);
        }
    }
    for(int col=0; col< num_cols; col++)
    {
        plotAt( num_rows-1, col)->close();
    }
    num_rows--;
    if( num_rows == 0){
        num_cols = 0;
    }

    updateLayout();
}


PlotMatrix::~PlotMatrix(){}

int PlotMatrix::rowsCount() const
{
    return num_rows;
}

int PlotMatrix::colsCount() const
{
    return num_cols;
}

int PlotMatrix::plotCount() const
{
    return num_rows*num_cols;
}

bool PlotMatrix::isColumnEmpty( int col )
{
    for (int r=0; r< layout->rowCount(); r++)
    {
        auto plot = plotAt(r, col);
        if( plot && ! plot->isEmpty() )  {
            return false;
        }
    }
    return true;
}

bool PlotMatrix::isRowEmpty(int row )
{
    for (int c=0; c< layout->columnCount(); c++)
    {
        auto plot = plotAt(row, c);
        if( plot && ! plot->isEmpty() )  {
            return false;
        }
    }
    return true;
}


PlotWidget* PlotMatrix::plotAt( int row, int column )
{
    QLayoutItem* item = layout->itemAtPosition(row,column);
    if(item) {
        PlotWidget* plot = static_cast<PlotWidget*>( item->widget() );
        return plot;
    }
    return NULL;
}

const PlotWidget* PlotMatrix::plotAt( int row, int column ) const
{
    QLayoutItem* item = layout->itemAtPosition(row,column);
    if(item) {
        PlotWidget* plot = static_cast<PlotWidget*>( item->widget() );
        return plot;
    }
    return NULL;
}

PlotWidget* PlotMatrix::plotAt( int index )
{
    QLayoutItem* item = layout->itemAt(index);
    if(item) {
        PlotWidget* plot = static_cast<PlotWidget*>( item->widget() );
        return plot;
    }
    return NULL;
}

const PlotWidget* PlotMatrix::plotAt( int index ) const
{
    QLayoutItem* item = layout->itemAt(index);
    if(item) {
        PlotWidget* plot = static_cast<PlotWidget*>( item->widget() );
        return plot;
    }
    return NULL;
}


void PlotMatrix::setAxisScale( int axis, int row, int col,
                               double min, double max, double step )
{
    PlotWidget *plt = plotAt( row, col );
    if ( plt )
    {
        //      plt->setAxisScale( axis, min, max, step );
        //      plt->updateAxes();
    }
}

QDomElement PlotMatrix::xmlSaveState( QDomDocument &doc )
{
    QDomElement element = doc.createElement("plotmatrix");

    element.setAttribute("rows", num_rows );
    element.setAttribute("columns", num_cols );

    for(int col = 0; col< num_cols; col++)
    {
        for(int row=0; row< num_rows; row++)
        {
            PlotWidget* plot = plotAt(row,col);
            QDomElement child = plot->xmlSaveState(doc);

            child.setAttribute("row", row);
            child.setAttribute("col", col);

            element.appendChild( child );
        }
    }
    return element;
}

bool PlotMatrix::xmlLoadState( QDomElement &plotmatrix )
{
    if( !plotmatrix.hasAttribute("rows") || !plotmatrix.hasAttribute("columns") )
    {
        qWarning() << "No [rows] or [columns] attribute in <plotmatrix> XML file!";
        return false;
    }
    int rows = plotmatrix.attribute("rows").toInt();
    int cols = plotmatrix.attribute("columns" ).toInt();

    while( rows > num_rows){ addRow(); }
    while( rows < num_rows){ removeRow( num_rows-1 );  }

    while( cols > num_cols){ addColumn();  }
    while( cols < num_cols){ removeColumn( num_cols-1 ); }

    QMessageBox::StandardButton load_answer = QMessageBox::Ok;

    QDomElement plot_element;
    for (  plot_element = plotmatrix.firstChildElement( "plot" )  ;
           !plot_element.isNull();
           plot_element = plot_element.nextSiblingElement( "plot" ) )
    {
        if( !plot_element.hasAttribute("row") || !plot_element.hasAttribute("col") )
        {
            qWarning() << "No [row] or [col] attribute in <plot> XML file!";
            return false;
        }
        int row = plot_element.attribute("row").toInt();
        int col = plot_element.attribute("col").toInt();

        bool success = plotAt(row,col)->xmlLoadState( plot_element, &load_answer ) ;
        if( !success )
        {
            return false;
        }
    }
    return true;
}



void PlotMatrix::updateLayout()
{
    /*    for ( int row = 0; row < rowsCount(); row++ )
    {
        alignAxes( row, QwtPlot::xBottom );
        alignScaleBorder( row, QwtPlot::yLeft );
    }

    for ( int col = 0; col < colsCount(); col++ )
    {
        alignAxes( col, QwtPlot::yLeft );
        alignScaleBorder( col, QwtPlot::xBottom );
    }*/
}

void PlotMatrix::replot()
{
    for ( unsigned i = 0; i< plotCount(); i++ )
    {
        PlotWidget *plot = plotAt(i);
        plot->replot();
    }
}


void PlotMatrix::setHorizontalLink(bool linked)
{
    _horizontal_link = linked;
}


void PlotMatrix::setName(const QString &new_name)
{
    _name = new_name;
}

const QString &PlotMatrix::name() const
{
    return _name;
}

QGridLayout *PlotMatrix::gridLayout()
{
    return layout;
}

void PlotMatrix::maximumZoomOutHorizontal()
{
    for ( unsigned i = 0; i< plotCount(); i++ )
    {
        PlotWidget *plot = plotAt(i);
        if( plot->isEmpty() == false)
        {
            plot->on_zoomOutHorizontal_triggered();
        }
    }
    replot();
}

void PlotMatrix::maximumZoomOutVertical()
{
    for ( unsigned i = 0; i < plotCount(); i++ )
    {
        PlotWidget *plot = plotAt(i);
        if( plot->isEmpty() == false)
        {
            plot->on_zoomOutVertical_triggered();
        }
    }
    replot();
}

void PlotMatrix::maximumZoomOut()
{
    for ( unsigned i = 0; i < plotCount(); i++ )
    {
        PlotWidget *plot = plotAt(i);
        if( plot->isEmpty() == false)
        {
            plot->zoomOut();
        }
    }
    replot();
}


void PlotMatrix::on_singlePlotScaleChanged(PlotWidget *modified_plot, QRectF new_range)
{
    if( _horizontal_link )
    {
        for ( unsigned i = 0; i< plotCount(); i++ )
        {
            PlotWidget *plot = plotAt(i);
            if( plot->isEmpty() == false && modified_plot != plot)
            {
                QRectF bound_act = plot->currentBoundingRect();
                bound_act.setLeft( new_range.left() );
                bound_act.setRight( new_range.right() );
                plot->setScale( bound_act, false );
                plot->replot();
            }
        }
    }

    emit undoableChange();
}

void PlotMatrix::alignAxes( int rowOrColumn, int axis )
{
    /*    if ( axis == QwtPlot::yLeft || axis == QwtPlot::yRight )
    {
        double maxExtent = 0;

        for ( int row = 0; row < rowsCount(); row++ )
        {
            QwtPlot *p = plotAt( row, rowOrColumn );
            if ( p )
            {
                QwtScaleWidget *scaleWidget = p->axisWidget( axis );

                QwtScaleDraw *sd = scaleWidget->scaleDraw();
                sd->setMinimumExtent( 0.0 );

                const double extent = sd->extent( scaleWidget->font() );
                if ( extent > maxExtent )
                    maxExtent = extent;
            }
        }

        for ( int row = 0; row < rowsCount(); row++ )
        {
            QwtPlot *p = plotAt( row, rowOrColumn );
            if ( p )
            {
                QwtScaleWidget *scaleWidget = p->axisWidget( axis );
                scaleWidget->scaleDraw()->setMinimumExtent( maxExtent );
            }
        }
    }
    else{
        double maxExtent = 0;

        for ( int col = 0; col < colsCount(); col++ )
        {
            QwtPlot *p = plotAt( rowOrColumn, col );
            if ( p )
            {
                QwtScaleWidget *scaleWidget = p->axisWidget( axis );

                QwtScaleDraw *sd = scaleWidget->scaleDraw();
                sd->setMinimumExtent( 0.0 );

                const double extent = sd->extent( scaleWidget->font() );
                if ( extent > maxExtent )
                    maxExtent = extent;
            }
        }

        for ( int col = 0; col < colsCount(); col++ )
        {
            QwtPlot *p = plotAt( rowOrColumn, col );
            if ( p )
            {
                QwtScaleWidget *scaleWidget = p->axisWidget( axis );
                scaleWidget->scaleDraw()->setMinimumExtent( maxExtent );
            }
        }
    }
}

void PlotMatrix::alignScaleBorder( int rowOrColumn, int axis )
{
    if ( axis == QwtPlot::yLeft || axis == QwtPlot::yRight )
    {
        for ( int col = 0; col < colsCount(); col++ )
        {
            QwtPlot *p = plotAt( rowOrColumn, col );
            if ( p )
                p->axisWidget( axis )->setMinBorderDist( 10, 10 );
        }
    }
    else if ( axis == QwtPlot::xTop | axis == QwtPlot::xBottom )
    {
        for ( int row = 0; row < rowsCount(); row++ )
        {
            QwtPlot *p = plotAt( row, rowOrColumn );
            if ( p )
                p->axisWidget( axis )->setMinBorderDist( 15, 15 );
        }
    }*/
}
