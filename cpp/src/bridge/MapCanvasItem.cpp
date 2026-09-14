#include "MapCanvasItem.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

static const int WORLD_W = 8192;
static const int WORLD_H = 4096;

MapCanvasItem::MapCanvasItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , m_worldImage(WORLD_W, WORLD_H, QImage::Format_RGBA8888)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptHoverEvents(true);
    m_worldImage.fill(QColor(36, 79, 117)); // ocean default
}

void MapCanvasItem::setBridge(WorldEditorBridge* bridge)
{
    if (m_bridge == bridge) return;

    if (m_bridge) {
        disconnect(m_bridge, nullptr, this, nullptr);
    }

    m_bridge = bridge;

    if (m_bridge) {
        connect(m_bridge, &WorldEditorBridge::regionDirty, this, &MapCanvasItem::onRegionDirty);
        connect(m_bridge, &WorldEditorBridge::worldLoaded, this, &MapCanvasItem::onWorldLoaded);
        onWorldLoaded();
    }

    emit bridgeChanged();
}

void MapCanvasItem::setZoom(qreal zoom)
{
    qreal clamped = qBound(0.05, zoom, 32.0);
    if (qFuzzyCompare(m_zoom, clamped)) return;
    m_zoom = clamped;
    emit zoomChanged();
    update();
}

void MapCanvasItem::setPanX(qreal x)
{
    if (qFuzzyCompare(m_panX, x)) return;
    m_panX = x;
    emit panChanged();
    update();
}

void MapCanvasItem::setPanY(qreal y)
{
    if (qFuzzyCompare(m_panY, y)) return;
    m_panY = y;
    emit panChanged();
    update();
}

void MapCanvasItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (oldGeometry.isEmpty() && !newGeometry.isEmpty()) {
        resetView();
    }
}

void MapCanvasItem::resetView()
{
    if (width() <= 0 || height() <= 0) return;

    qreal scaleX = width() / (qreal)WORLD_W;
    qreal scaleY = height() / (qreal)WORLD_H;
    m_zoom = qMin(scaleX, scaleY);
    m_panX = (width() - (WORLD_W * m_zoom)) * 0.5;
    m_panY = (height() - (WORLD_H * m_zoom)) * 0.5;

    emit zoomChanged();
    emit panChanged();
    update();
}

void MapCanvasItem::centerOn(qreal worldX, qreal worldY)
{
    m_panX = (width() * 0.5) - (worldX * m_zoom);
    m_panY = (height() * 0.5) - (worldY * m_zoom);
    emit panChanged();
    update();
}

void MapCanvasItem::zoomIn()
{
    QPointF center(width() * 0.5, height() * 0.5);
    QPointF worldCenter = screenToWorld(center);
    setZoom(m_zoom * 1.3);
    centerOn(worldCenter.x(), worldCenter.y());
}

void MapCanvasItem::zoomOut()
{
    QPointF center(width() * 0.5, height() * 0.5);
    QPointF worldCenter = screenToWorld(center);
    setZoom(m_zoom / 1.3);
    centerOn(worldCenter.x(), worldCenter.y());
}

QPointF MapCanvasItem::screenToWorld(const QPointF& screenPos) const
{
    return QPointF((screenPos.x() - m_panX) / m_zoom, (screenPos.y() - m_panY) / m_zoom);
}

QPointF MapCanvasItem::worldToScreen(const QPointF& worldPos) const
{
    return QPointF((worldPos.x() * m_zoom) + m_panX, (worldPos.y() * m_zoom) + m_panY);
}

void MapCanvasItem::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_zoom < 1.0);

    // Compute visible destination rect
    QRectF targetRect(m_panX, m_panY, WORLD_W * m_zoom, WORLD_H * m_zoom);
    QRectF sourceRect(0, 0, WORLD_W, WORLD_H);

    painter->drawImage(targetRect, m_worldImage, sourceRect);
}

void MapCanvasItem::onRegionDirty(int minX, int minY, int maxX, int maxY)
{
    if (!m_bridge || !m_bridge->handle()) return;

    int w = maxX - minX + 1;
    int h = maxY - minY + 1;
    if (w <= 0 || h <= 0) return;

    QByteArray buffer(w * h * 4, Qt::Uninitialized);
    bool ok = matalas_world_render_rect(
        m_bridge->handle(),
        minX, minY, maxX, maxY,
        reinterpret_cast<uint8_t*>(buffer.data()),
        buffer.size()
    );

    if (ok) {
        const uchar* srcData = reinterpret_cast<const uchar*>(buffer.constData());
        int srcLineBytes = w * 4;

        for (int y = 0; y < h; ++y) {
            uchar* destLine = m_worldImage.scanLine(minY + y) + (minX * 4);
            memcpy(destLine, srcData + (y * srcLineBytes), srcLineBytes);
        }

        update();
    }
}

void MapCanvasItem::onWorldLoaded()
{
    onRegionDirty(0, 0, WORLD_W - 1, WORLD_H - 1);
}

void MapCanvasItem::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = event->position();
    QPointF worldPosF = screenToWorld(m_lastMousePos);
    int wx = qBound(0, (int)worldPosF.x(), WORLD_W - 1);
    int wy = qBound(0, (int)worldPosF.y(), WORLD_H - 1);

    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && m_bridge) {
        int tool = m_bridge->activeTool();
        if (tool == 2) { // Fill
            m_bridge->fillAt(wx, wy);
        } else if (tool == 3) { // Picker
            m_bridge->pickAt(wx, wy);
        } else { // Brush / Eraser
            m_isPainting = true;
            m_lastWorldPos = QPoint(wx, wy);
            m_bridge->paintAt(wx, wy);
        }
        event->accept();
    }
}

void MapCanvasItem::mouseMoveEvent(QMouseEvent* event)
{
    QPointF pos = event->position();
    QPointF delta = pos - m_lastMousePos;
    m_lastMousePos = pos;

    QPointF worldPosF = screenToWorld(pos);
    int wx = qBound(0, (int)worldPosF.x(), WORLD_W - 1);
    int wy = qBound(0, (int)worldPosF.y(), WORLD_H - 1);
    emit cursorWorldCoordsChanged(wx, wy);

    if (m_isPanning) {
        m_panX += delta.x();
        m_panY += delta.y();
        emit panChanged();
        update();
        event->accept();
        return;
    }

    if (m_isPainting && m_bridge) {
        strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx, wy);
        m_lastWorldPos = QPoint(wx, wy);
        event->accept();
    }
}

void MapCanvasItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        event->accept();
    } else if (event->button() == Qt::LeftButton) {
        m_isPainting = false;
        event->accept();
    }
}

void MapCanvasItem::wheelEvent(QWheelEvent* event)
{
    QPointF mousePos = event->position();
    QPointF worldBefore = screenToWorld(mousePos);

    qreal angle = event->angleDelta().y();
    qreal factor = (angle > 0) ? 1.2 : 0.83333;

    qreal newZoom = qBound(0.05, m_zoom * factor, 32.0);
    if (!qFuzzyCompare(newZoom, m_zoom)) {
        m_zoom = newZoom;
        // Keep cursor position stable on zoom
        m_panX = mousePos.x() - (worldBefore.x() * m_zoom);
        m_panY = mousePos.y() - (worldBefore.y() * m_zoom);

        emit zoomChanged();
        emit panChanged();
        update();
    }

    event->accept();
}

void MapCanvasItem::strokeLine(int x0, int y0, int x1, int y1)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int currX = x0;
    int currY = y0;

    int radius = m_bridge->brushRadius();
    int step = qMax(1, radius / 2);
    int counter = 0;

    while (true) {
        if (counter % step == 0) {
            m_bridge->paintAt(currX, currY);
        }
        counter++;

        if (currX == x1 && currY == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            currX += sx;
        }
        if (e2 < dx) {
            err += dx;
            currY += sy;
        }
    }
}
