#ifndef SCALEIMAGE_GRAPHICSVIEW_H
#define SCALEIMAGE_GRAPHICSVIEW_H

#include <QObject>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QMenu>

// Default zoom factors
#define         DEFAULT_ZOOM_FACTOR             1.15
#define         DEFAULT_ZOOM_CTRL_FACTOR        1.01

/*!
 * \brief The ScaleImage_GraphicsView class     This class has been designed to display an image
 *                                              The image can be zoomed using mouse wheel
 */
class ScaleImage_GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    ScaleImage_GraphicsView(QWidget * parent=0);

    /*!
     * \brief setImage                          Set the image in the widget
     * \param image                             QImage to display
     */
    void setImage(const QImage & image);

    /*!
     * \brief setZoomFactor                     Set the zoom factor when the CTRL key is not pressed
     * \param factor                            zoom factor (>1)
     */
    void setZoomFactor(const double factor);

    /*!
     * \brief setZoomCtrlFactor                 Set the zoom factor when the CTRL key is pressed
     * \param factor                            zoom factor (>1)
     */
    void setZoomCtrlFactor(const double factor);

protected:
    /*!
     * \brief mousePressEvent                   Overload this function to process mouse button pressed
     * \param event                             mouse event
     */
    virtual void mousePressEvent(QMouseEvent *event);

    /*!
     * \brief wheelEvent                        Overload this function to process mouse wheel event
     * \param event                             mouse wheel event
     */
    virtual void wheelEvent(QWheelEvent *event);

    /*!
     * \brief mouseMoveEvent                    Overload this function to process mouse moves
     * \param event                             mouse move event
     */
    virtual void mouseMoveEvent(QMouseEvent *event);

private:

    // Scene where the image is drawn
    QGraphicsScene* scene;
    // Pixmap item containing the image
    QGraphicsPixmapItem* pixmapItem;
    // Size of the current image
    QSize imageSize;
    // Zoom factor
    double zoomFactor;
    // Zoom factor when the CTRL key is pressed
    double zoomCtrlFactor;
    // Current pixmap
    QPixmap pixmap;

signals:
    /*!
     * \brief mouseMoved                        Fires to notify the new mouse location to the corresponding slot
     * \param mousePos                          Current mouse location
     */
    void mouseMoved(QPointF mousePos);

    /*!
     * \brief mousePressed                      Fires to notify the mouse click
     */
    void mousePressed();
};

#endif // SCALEIMAGE_GRAPHICSVIEW_H
