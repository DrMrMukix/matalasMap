#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include <QPointF>
#include <QHash>
#include <QSharedPointer>
#include "WorldEditorBridge.h"

class QSvgRenderer;

class MapCanvasItem : public QQuickPaintedItem {
    Q_OBJECT

    Q_PROPERTY(WorldEditorBridge* bridge READ bridge WRITE setBridge NOTIFY bridgeChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(qreal panX READ panX WRITE setPanX NOTIFY panChanged)
    Q_PROPERTY(qreal panY READ panY WRITE setPanY NOTIFY panChanged)

public:
    explicit MapCanvasItem(QQuickItem* parent = nullptr);

    WorldEditorBridge* bridge() const { return m_bridge; }
    void setBridge(WorldEditorBridge* bridge);

    qreal zoom() const { return m_zoom; }
    void setZoom(qreal zoom);

    qreal panX() const { return m_panX; }
    void setPanX(qreal x);

    qreal panY() const { return m_panY; }
    void setPanY(qreal y);

    void paint(QPainter* painter) override;

public slots:
    void resetView();
    void centerOn(qreal worldX, qreal worldY);
    void zoomIn();
    void zoomOut();

signals:
    void bridgeChanged();
    void zoomChanged();
    void panChanged();
    void cursorWorldCoordsChanged(int worldX, int worldY);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void touchEvent(QTouchEvent* event) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private slots:
    void onRegionDirty(int minX, int minY, int maxX, int maxY);
    void onWorldLoaded();

private:
    QPointF screenToWorld(const QPointF& screenPos) const;
    QPointF worldToScreen(const QPointF& worldPos) const;
    bool isWorldPosValid(const QPointF& worldPos) const;
    void strokeLine(int x0, int y0, int x1, int y1);

    WorldEditorBridge* m_bridge = nullptr;

    qreal m_zoom = 1.0;
    qreal m_panX = 0.0;
    qreal m_panY = 0.0;

    bool m_isPainting = false;
    bool m_isPanning = false;
    bool m_isHovering = false;
    QPointF m_currentScreenPos;
    QPointF m_lastMousePos;
    QPoint m_lastWorldPos;

    bool m_pinchActive = false;
    qreal m_initialPinchDist = 0.0;
    qreal m_initialPinchZoom = 1.0;
    QPointF m_lastPinchMid;

    QHash<QString, QSharedPointer<QSvgRenderer>> m_svgCache;
};
