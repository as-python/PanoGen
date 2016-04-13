#include "scaleimage_graphicsview.h"

ScaleImage_GraphicsView::ScaleImage_GraphicsView( QWidget * parent ) : QGraphicsView(parent)
{
    zoomFactor=DEFAULT_ZOOM_FACTOR;
    zoomCtrlFactor=DEFAULT_ZOOM_CTRL_FACTOR;

    scene = new QGraphicsScene();
    this->setMouseTracking(true);

    this->setScene(scene);

    // When zooming, the view stay centered over the mouse
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    pixmapItem = scene->addPixmap(pixmap);

}



void ScaleImage_GraphicsView::setImage(const QImage &image)
{
    // Update the pixmap in the scene
    pixmap = QPixmap::fromImage(image);
    pixmapItem->setPixmap(pixmap);

    // Resize the scene (needed is the new image is smaller)
    scene->setSceneRect(QRect (QPoint(0,0),image.size()));

    // Store the image size
    imageSize = image.size();
}

void ScaleImage_GraphicsView::setZoomFactor(const double factor)
{
    zoomFactor=factor;
}

void ScaleImage_GraphicsView::setZoomCtrlFactor(const double factor)
{
    zoomCtrlFactor=factor;
}

void ScaleImage_GraphicsView::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed();
    QGraphicsView::mousePressEvent(event);
}

void ScaleImage_GraphicsView::wheelEvent(QWheelEvent *event)
{    

    // When zooming, the view stay centered over the mouse
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    double factor = (event->modifiers() & Qt::ControlModifier) ? zoomCtrlFactor : zoomFactor;
    if(event->delta() > 0)
        // Zoom in
        scale(factor, factor);
    else
        // Zooming out
        scale(1.0 / factor, 1.0 / factor);

    // The event is processed
    event->accept();
}

void ScaleImage_GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    // Get the coordinates of the mouse in the scene
    QPointF imagePoint = mapToScene(QPoint(event->x(), event->y() ));
    // Call the parent's function (for dragging)
    QGraphicsView::mouseMoveEvent(event);

    emit mouseMoved(imagePoint);
}
