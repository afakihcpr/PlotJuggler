#include "plotwidget.h"
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>

#include <QAction>
#include <QMessageBox>
#include <QMenu>
#include <limits>
#include "removecurvedialog.h"
#include "curvecolorpick.h"
#include <QApplication>
#include <set>
#include <memory>
#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>
#include <QtCharts/QValueAxis>

using namespace QtCharts;

PlotWidget::PlotWidget(PlotDataMap *datamap, QWidget *parent):
    QChartView(parent),
    //_zoomer( 0 ),
    // _magnifier(0 ),
    // _panner( 0 ),
    // _tracker ( 0 ),
    //  _legend( 0 ),
    _mapped_data( datamap )
{

    this->setChart( new QChart() );

    this->setAcceptDrops( true );
    this->setMinimumWidth( 100 );
    this->setMinimumHeight( 100 );

    this->sizePolicy().setHorizontalPolicy( QSizePolicy::Expanding);
    this->sizePolicy().setVerticalPolicy( QSizePolicy::Expanding);

    this->setRenderHint(QPainter::Antialiasing);

    this->chart()->installEventFilter( this );
    //  TODO connect(_zoomer, SIGNAL(zoomed(QRectF)), this, SLOT(on_externallyResized(QRectF)) );
    // TODO connect(_magnifier, SIGNAL(rescaled(QRectF)), this, SLOT(on_externallyResized(QRectF)) );
    // TODO connect(_magnifier, SIGNAL(rescaled(QRectF)), this, SLOT(replot()) );

    // TODO _panner->setMouseButton(  Qt::MiddleButton, Qt::NoModifier);

    // TODO  connect( canvas, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(canvasContextMenuTriggered(QPoint)) );
    //-------------------------

    chart()->legend()->hide();

    QValueAxis* axis_x = new QValueAxis(this);
    axis_x->setRange(0, 1);
    QValueAxis* axis_y = new QValueAxis(this);
    axis_y->setRange(0, 1);

    chart()->setAxisX( axis_x );
    chart()->setAxisY( axis_y );

    chart()->setVisible( true );

    buildActions();
    buildLegend();

    // TODO   this->chart()->setMouseTracking(true);
    // TODO   this->chart()->installEventFilter(this);

    connect( chart()->scene(), &QGraphicsScene::changed, this, &PlotWidget::replot  );

    _replot_timer.setInterval(1);
    _replot_timer.setSingleShot(true);
    QObject::connect(&_replot_timer, &QTimer::timeout, this, &PlotWidget::replotImpl);

}

void PlotWidget::buildActions()
{
    _action_removeCurve = new QAction(tr("&Remove curves"), this);
    _action_removeCurve->setStatusTip(tr("Remove one or more curves from this plot"));
    connect(_action_removeCurve, SIGNAL(triggered()), this, SLOT(launchRemoveCurveDialog()));

    QIcon iconDelete;
    iconDelete.addFile(QStringLiteral(":/icons/resources/checkboxalt.png"), QSize(26, 26), QIcon::Normal, QIcon::Off);
    _action_removeAllCurves = new QAction(tr("&Remove all curves"), this);
    _action_removeAllCurves->setIcon(iconDelete);
    connect(_action_removeAllCurves, SIGNAL(triggered()), this, SLOT(detachAllCurves()));
    connect(_action_removeAllCurves, SIGNAL(triggered()), this, SIGNAL(undoableChange()) );

    QIcon iconColors;
    iconColors.addFile(QStringLiteral(":/icons/resources/office_chart_lines.png"), QSize(26, 26), QIcon::Normal, QIcon::Off);
    _action_changeColors = new QAction(tr("&Change colors"), this);
    _action_changeColors->setIcon(iconColors);
    _action_changeColors->setStatusTip(tr("Change the color of the curves"));
    connect(_action_changeColors, SIGNAL(triggered()), this, SLOT(on_changeColor_triggered()));


    QIcon iconPoints;
    iconPoints.addFile(QStringLiteral(":/icons/resources/line_chart_32px.png"), QSize(26, 26), QIcon::Normal, QIcon::Off);
    _action_showPoints = new QAction(tr("&Show lines and points"), this);
    _action_showPoints->setIcon(iconPoints);
    _action_showPoints->setCheckable( true );
    _action_showPoints->setChecked( false );
    connect(_action_showPoints, SIGNAL(triggered(bool)), this, SLOT(on_showPoints_triggered(bool)));

    QIcon iconZoomH;
    iconZoomH.addFile(QStringLiteral(":/icons/resources/resize_horizontal.png"), QSize(26, 26), QIcon::Normal, QIcon::Off);
    _action_zoomOutHorizontally = new QAction(tr("&Zoom Out Horizontally"), this);
    _action_zoomOutHorizontally->setIcon(iconZoomH);
    connect(_action_zoomOutHorizontally, SIGNAL(triggered()), this, SLOT(on_zoomOutHorizontal_triggered()));
    connect(_action_zoomOutHorizontally, SIGNAL(triggered()), this, SIGNAL(undoableChange()) );

    QIcon iconZoomV;
    iconZoomV.addFile(QStringLiteral(":/icons/resources/resize_vertical.png"), QSize(26, 26), QIcon::Normal, QIcon::Off);
    _action_zoomOutVertically = new QAction(tr("&Zoom Out Vertically"), this);
    _action_zoomOutVertically->setIcon(iconZoomV);
    connect(_action_zoomOutVertically, SIGNAL(triggered()), this, SLOT(on_zoomOutVertical_triggered()));
    connect(_action_zoomOutVertically, SIGNAL(triggered()), this, SIGNAL(undoableChange()) );

}

void PlotWidget::buildLegend()
{

}



PlotWidget::~PlotWidget()
{

}

bool PlotWidget::addCurve(const QString &name, bool do_replot)
{
    auto it = _mapped_data->numeric.find( name.toStdString() );
    if( it == _mapped_data->numeric.end())
    {
        return false;
    }

    if( _curve_list.find(name) != _curve_list.end())
    {
        return false;
    }

    auto series = std::shared_ptr<QLineSeries>( new QLineSeries(this) );
    series->setUseOpenGL(true);
    PlotDataPtr data   = it->second;

    QPen pen = series->pen();
    pen.setWidthF( 0.5 );
    series->setPen( pen );

    for(int i=0; i< data->size(); i++ )
    {
        auto point = data->at(i);
        series->append( point.x, point.y );
    }

    int red, green,blue;
    data->getColorHint(& red, &green, &blue);
    series->setColor( QColor( red, green, blue) );

    chart()->addSeries( series.get() );

    PlotCurve curve = { series, data};
    _curve_list.insert( std::make_pair(name, curve ));


    auto rangeX = maximumRangeX();
    auto rangeY = maximumRangeY();

    this->chart()->createDefaultAxes();
    this->chart()->axisX()->setRange( rangeX.first, rangeX.second );
    this->chart()->axisY()->setRange( rangeY.first, rangeY.second );

    if( do_replot )
    {
        replot();
    }

    return true;
}

void PlotWidget::removeCurve(const QString &name)
{
    auto it = _curve_list.find(name);
    if( it != _curve_list.end() )
    {
        auto curve = it->second;
        chart()->removeSeries( curve.series.get() );
        replot();
        _curve_list.erase( it );
    }
}

bool PlotWidget::isEmpty()
{
    return _curve_list.empty();
}



void PlotWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QChartView::dragEnterEvent(event);

    const QMimeData *mimeData = event->mimeData();
    QStringList mimeFormats = mimeData->formats();
    foreach(QString format, mimeFormats)
    {
        QByteArray encoded = mimeData->data( format );
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        if( format.contains( "qabstractitemmodeldatalist") )
        {
            event->acceptProposedAction();
        }
        if( format.contains( "plot_area")  )
        {
            QString source_name;
            stream >> source_name;

            if(QString::compare( windowTitle(),source_name ) != 0 ){
                event->acceptProposedAction();
            }
        }
    }
}
void PlotWidget::dragMoveEvent(QDragMoveEvent *)
{

}


void PlotWidget::dropEvent(QDropEvent *event)
{
    QChartView::dropEvent(event);

    const QMimeData *mimeData = event->mimeData();
    QStringList mimeFormats = mimeData->formats();

    foreach(QString format, mimeFormats)
    {
        QByteArray encoded = mimeData->data( format );
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        if( format.contains( "qabstractitemmodeldatalist") )
        {
            bool plot_added = false;
            while (!stream.atEnd())
            {
                int row, col;
                QMap<int,  QVariant> roleDataMap;

                stream >> row >> col >> roleDataMap;

                QString curve_name = roleDataMap[0].toString();
                addCurve( curve_name );
                plot_added = true;
            }
            if( plot_added )
            {
                emit undoableChange();
            }
        }
        if( format.contains( "plot_area") )
        {
            QString source_name;
            stream >> source_name;
            PlotWidget* source_plot = static_cast<PlotWidget*>( event->source() );
            emit swapWidgetsRequested( source_plot, this );
        }
    }
}

void PlotWidget::detachAllCurves()
{
    this->chart()->removeAllSeries();
    _curve_list.clear();
    replot();
}

QDomElement PlotWidget::xmlSaveState( QDomDocument &doc)
{
    QDomElement plot_el = doc.createElement("plot");

    QDomElement range_el = doc.createElement("range");
    QRectF rect = this->currentBoundingRect();
    range_el.setAttribute("bottom", QString::number(rect.bottom()) );
    range_el.setAttribute("top", QString::number(rect.top()) );
    range_el.setAttribute("left", QString::number(rect.left()) );
    range_el.setAttribute("right", QString::number(rect.right()) );
    plot_el.appendChild(range_el);

    for(auto it=_curve_list.begin(); it != _curve_list.end(); ++it)
    {
        QString name = it->first;
        auto curve = it->second;
        QDomElement curve_el = doc.createElement("curve");
        curve_el.setAttribute( "name",name);

        QColor color = curve.series->color();

        curve_el.setAttribute( "R", color.red());
        curve_el.setAttribute( "G", color.green());
        curve_el.setAttribute( "B", color.blue());

        plot_el.appendChild(curve_el);
    }
    return plot_el;
}

bool PlotWidget::xmlLoadState(QDomElement &plot_widget, QMessageBox::StandardButton* answer)
{
    QDomElement curve;

    std::set<QString> added_curve_names;

    for (  curve = plot_widget.firstChildElement( "curve" )  ;
           !curve.isNull();
           curve = curve.nextSiblingElement( "curve" ) )
    {
        QString curve_name = curve.attribute("name");
        int R = curve.attribute("R").toInt();
        int G = curve.attribute("G").toInt();
        int B = curve.attribute("B").toInt();
        QColor color(R,G,B);

        if(  _mapped_data->numeric.find(curve_name.toStdString()) != _mapped_data->numeric.end() )
        {
            addCurve(curve_name, false);
            _curve_list[curve_name].series->setColor( color );
            added_curve_names.insert(curve_name );
        }
        else{
            if( *answer !=  QMessageBox::YesToAll)
            {
                *answer = QMessageBox::question(0,
                                                tr("Warning"),
                                                tr("Can't find the curve with name %1.\n Do you want to ignore it? ").arg(curve_name),
                                                QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Abort ,
                                                QMessageBox::Abort );
            }

            if( *answer ==  QMessageBox::Yes || *answer ==  QMessageBox::YesToAll) {
                continue;
            }

            if( *answer ==  QMessageBox::Abort) {
                return false;
            }
        }
    }

    bool curve_removed = true;

    while( curve_removed)
    {
        curve_removed = false;
        for(auto& it: _curve_list)
        {
            QString curve_name = it.first;
            if( added_curve_names.find( curve_name ) == added_curve_names.end())
            {
                removeCurve( curve_name );
                curve_removed = true;
                break;
            }
        }
    }


    QDomElement rectangle =  plot_widget.firstChildElement( "range" );
    QRectF rect;
    rect.setBottom( rectangle.attribute("bottom").toDouble());
    rect.setTop( rectangle.attribute("top").toDouble());
    rect.setLeft( rectangle.attribute("left").toDouble());
    rect.setRight( rectangle.attribute("right").toDouble());

    this->setScale( rect, false);

    return true;
}


QRectF PlotWidget::currentBoundingRect()
{
     //  return this->chart()->sceneBoundingRect();
   return this->chart()->boundingRect();

    /* QRectF rect;
    rect.setBottom( this->canvasMap( yLeft ).s1() );
    rect.setTop( this->canvasMap( yLeft ).s2() );

    rect.setLeft( this->canvasMap( xBottom ).s1() );
    rect.setRight( this->canvasMap( xBottom ).s2() );*/

    // return rect;
}

// TODO
/*
CurveTracker *PlotWidget::tracker()
{
    return _tracker;
}*/

void PlotWidget::setScale(QRectF rect, bool emit_signal)
{
    this->chart()->axisX()->setRange( rect.left(), rect.right());
    this->chart()->axisY()->setRange( rect.bottom(), rect.top());

    if( emit_signal) {
        emit rectChanged(this, rect);
    }
}

void PlotWidget::activateLegent(bool activate)
{
    // TODO if( activate ) _legend->attach(this);
    // TODO else           _legend->detach();
}


void PlotWidget::setAxisScale(int axisId, double min, double max, double step)
{
    // TODO if (axisId == xBottom)
    {
        for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
        {
            // TODO PlotCurve* data = static_cast<PlotCurve*>( it->second->data() );
            // TODO data->setSubsampleFactor( );
        }
    }

    // TODO  QChartView::setAxisScale( axisId, min, max, step);
}

std::pair<float,float> PlotWidget::maximumRangeX()
{
    float left   = std::numeric_limits<double>::max();
    float right  = std::numeric_limits<double>::min();

    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        PlotDataPtr data = ( it->second.data );
        auto range_X = data->getRangeX();

        if( left  > range_X.min )    left  = range_X.min;
        if( right < range_X.max )    right = range_X.max;
    }

    /* if( fabs(top-bottom) < 1e-10  )
    {
        max_bounding = QRectF(left, top+0.01,  right - left, -0.02 ) ;
    }
    else{
        double height = bottom - top;
        max_bounding = QRectF(left, top - height*0.05,  right - left, height*1.1 ) ;
    }*/

    // TODO _magnifier->setAxisLimits( xBottom, left, right);

    return std::make_pair( left, right);
}

std::pair<float,float>  PlotWidget::maximumRangeY(bool current_canvas)
{
    float top    = std::numeric_limits<float>::min();
    float bottom = std::numeric_limits<float>::max();

    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        PlotDataPtr data = ( it->second.data );

        float min_X, max_X;
        // TODO  if( current_canvas )
        /* {
            min_X = this->canvasMap( QChartView::xBottom ).s1();
            max_X = this->canvasMap( QChartView::xBottom ).s2();
        }
        else*/
        {
            auto range_X = maximumRangeX();
            min_X = range_X.first;
            max_X = range_X.second;
        }

        size_t X0 = data->getIndexFromX( min_X );
        size_t X1 = data->getIndexFromX( max_X );

        auto range_Y = data->getRangeY(X0, X1);

        if( top <    range_Y.max )    top    = range_Y.max;
        if( bottom > range_Y.min )    bottom = range_Y.min;
    }

    // TODO _magnifier->setAxisLimits( yLeft, bottom, top);
    return std::make_pair( bottom,  top);
}



void PlotWidget::replotImpl()
{
    // TODO if( _zoomer )
    // TODO      _zoomer->setZoomBase( false );

    /* if(_tracker ) {
        _tracker->refreshPosition( );
    }*/
    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        PlotDataPtr data = it->second.data;
        auto series = it->second.series;
        const int N = data->size();

        QVector<QPointF> new_data;
        new_data.resize( N );

        for ( int i=0; i< N; i++)
        {
            auto point = data->at(i);
            new_data[i] = QPointF( point.x, point.y);
        }
        series->replace( new_data );
    }


    if ( !_fps_timeStamp.isValid() )
    {
        _fps_timeStamp.start();
        _fps_counter = 0;
    }
    else{
        _fps_counter++;

        const int elapsed = _fps_timeStamp.elapsed() ;
        if ( elapsed >= 1000 )
        {

            QString fps( QString::number( ( (1000 *_fps_counter) / elapsed ) ) );
            this->chart()->setTitle( fps );

            _fps_counter = 0;
            _fps_timeStamp.restart();
        }
    }
}

void PlotWidget::launchRemoveCurveDialog()
{
    RemoveCurveDialog* dialog = new RemoveCurveDialog(this);
    unsigned prev_curve_count = _curve_list.size();

    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        dialog->addCurveName( it->first );
    }

    dialog->exec();

    if( prev_curve_count != _curve_list.size() )
    {
        emit undoableChange();
    }
}

void PlotWidget::on_changeColor_triggered()
{
    std::map<QString,QColor> color_by_name;

    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        const QString& curve_name = it->first;
        auto curve = it->second;
        color_by_name.insert(std::make_pair( curve_name, curve.series->color() ));
    }

    CurveColorPick* dialog = new CurveColorPick(&color_by_name, this);
    dialog->exec();

    bool modified = false;

    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        const QString& curve_name = it->first;
        auto curve = it->second;
        QColor new_color = color_by_name[curve_name];
        if( curve.series->color() != new_color)
        {
            // TODO curve->setPen( color_by_name[curve_name], 1.0 );
            modified = true;
        }
    }
    if( modified){
        emit undoableChange();
    }
}

void PlotWidget::on_showPoints_triggered(bool checked)
{
    for(auto it = _curve_list.begin(); it != _curve_list.end(); ++it)
    {
        auto curve = it->second;
        if( checked )
        {
            // TODO curve->setStyle( QLineSeries::LinesAndDots);
        }
        else{
            // TODO curve->setStyle( QLineSeries::Lines);
        }
    }
    replot();
}

void PlotWidget::on_externallyResized(QRectF rect)
{
    emit rectChanged( this, rect);
}

void PlotWidget::replot()
{
 //   qDebug() << "on_sceneUpdated " << QTime::currentTime().toString();
    _replot_timer.start();
}


void PlotWidget::zoomOut()
{
    QRectF rect = currentBoundingRect();
    auto rangeX = maximumRangeX();

    rect.setLeft( rangeX.first );
    rect.setRight( rangeX.second );

    auto rangeY = maximumRangeY( false );

    rect.setBottom( rangeY.first );
    rect.setTop( rangeY.second );
    this->setScale(rect, false);
}

void PlotWidget::on_zoomOutHorizontal_triggered()
{
    QRectF act = currentBoundingRect();
    auto rangeX = maximumRangeX();

    act.setLeft( rangeX.first );
    act.setRight( rangeX.second );
    this->setScale(act);
}

void PlotWidget::on_zoomOutVertical_triggered()
{
    QRectF act = currentBoundingRect();
    auto rangeY = maximumRangeY( true );

    act.setBottom( rangeY.first );
    act.setTop( rangeY.second );
    this->setScale(act);
}

void PlotWidget::canvasContextMenuTriggered(const QPoint &pos)
{
    QMenu menu(this);
    menu.addAction(_action_removeCurve);
    menu.addAction(_action_removeAllCurves);
    menu.addSeparator();
    menu.addAction(_action_changeColors);
    menu.addAction(_action_showPoints);
    menu.addSeparator();
    menu.addAction(_action_zoomOutHorizontally);
    menu.addAction(_action_zoomOutVertically);

    _action_removeCurve->setEnabled( ! _curve_list.empty() );
    _action_removeAllCurves->setEnabled( ! _curve_list.empty() );
    _action_changeColors->setEnabled(  ! _curve_list.empty() );

    menu.exec( this->mapToGlobal(pos) ); // TODO ... will it work?
}

void PlotWidget::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton)
    {
        if (event->modifiers() & Qt::ControlModifier )
        {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;

            QByteArray data;
            QDataStream dataStream(&data, QIODevice::WriteOnly);

            dataStream << this->windowTitle();

            mimeData->setData("plot_area", data );
            drag->setMimeData(mimeData);
            drag->exec();
        }
        else if( event->modifiers() == Qt::NoModifier)
        {
            QApplication::setOverrideCursor(QCursor(QPixmap(":/icons/resources/zoom_in_32px.png")));
        }
    }

    if( event->button() == Qt::MiddleButton && event->modifiers() == Qt::NoModifier)
    {
        QApplication::setOverrideCursor(QCursor(QPixmap(":/icons/resources/move.png")));
    }

    QChartView::mousePressEvent(event);
}

void PlotWidget::mouseReleaseEvent(QMouseEvent *event )
{
    QApplication::restoreOverrideCursor();
    QChartView::mouseReleaseEvent(event);
}

bool PlotWidget::eventFilter(QObject *obj, QEvent *event)
{
    static bool isPressed = true;

    if ( event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouse_event = (QMouseEvent *)event;

        if( mouse_event->button() == Qt::LeftButton &&
                (mouse_event->modifiers() & Qt::ShiftModifier) )
        {
            isPressed = true;
            const QPoint point = mouse_event->pos();
            QPointF pointF = chart()->mapToValue( point);
            emit trackerMoved(pointF);
        }
    }

    if ( event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouse_event = (QMouseEvent *)event;

        if( mouse_event->button() == Qt::LeftButton )
        {
            isPressed = false;
        }
    }

    if ( event->type() == QEvent::MouseMove )
    {
        // special processing for mouse move
        QMouseEvent *mouse_event = (QMouseEvent *)event;

        if ( isPressed && mouse_event->modifiers() & Qt::ShiftModifier )
        {
            const QPoint point = mouse_event->pos();
            QPointF pointF = this->chart()->mapToValue( point );

            emit trackerMoved(pointF);
        }
    }

    return QChartView::eventFilter( obj, event );
}

