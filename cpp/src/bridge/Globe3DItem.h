#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include "WorldEditorBridge.h"

class Globe3DItem : public QQuickPaintedItem {
    Q_OBJECT

    Q_PROPERTY(WorldEditorBridge* bridge READ bridge WRITE setBridge NOTIFY bridgeChanged)
    Q_PROPERTY(qreal yaw READ yaw WRITE setYaw NOTIFY yawChanged)
    Q_PROPERTY(qreal pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

public:
    explicit Globe3DItem(QQuickItem* parent = nullptr);

    WorldEditorBridge* bridge() const { return m_bridge; }
    void setBridge(WorldEditorBridge* bridge);

    qreal yaw() const { return m_yaw; }
    void setYaw(qreal y);

    qreal pitch() const { return m_pitch; }
    void setPitch(qreal p);

    qreal zoom() const { return m_zoom; }
    void setZoom(qreal z);

    void paint(QPainter* painter) override;

public slots:
    void resetView();
    void zoomIn();
    void zoomOut();

signals:
    void bridgeChanged();
    void yawChanged();
    void pitchChanged();
    void zoomChanged();
    void cursorWorldCoordsChanged(int wx, int wy);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void touchEvent(QTouchEvent* event) override;

private slots:
    void onRegionDirty(int minX, int minY, int maxX, int maxY);
    void onWorldLoaded();

private:
    bool raycastSphere(const QPointF& screenPos, int& outWx, int& outWy) const;
    void strokeLine(int x0, int y0, int x1, int y1);

    WorldEditorBridge* m_bridge = nullptr;

    qreal m_yaw = 0.0;     // radians
    qreal m_pitch = 0.2;   // radians
    qreal m_zoom = 1.0;

    bool m_isRotating = false;
    bool m_isPainting = false;
    QPointF m_lastMousePos;
    QPoint m_lastWorldPos;

    bool m_pinchActive = false;
    qreal m_initialPinchDist = 0.0;
    qreal m_initialPinchZoom = 1.0;
    QPointF m_lastPinchMid;
};
