#include "MapCanvasItem.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTouchEvent>
#include <QLineF>
#include <QCursor>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <cmath>

static const int WORLD_W = 8192;
static const int WORLD_H = 4096;

MapCanvasItem::MapCanvasItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptTouchEvents(true);
    setAcceptHoverEvents(true);
    setCursor(Qt::CrossCursor);
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
        connect(m_bridge, &WorldEditorBridge::displayModeChanged, this, [this]() { update(); });
        connect(m_bridge, &WorldEditorBridge::activeToolChanged, this, [this]() {
            if (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand) {
                setCursor(Qt::OpenHandCursor);
            } else {
                setCursor(Qt::CrossCursor);
            }
        });
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
    qreal scaledH = WORLD_H * m_zoom;
    qreal clampedY = y;
    if (height() > 0 && scaledH > 0) {
        if (scaledH <= height()) {
            clampedY = (height() - scaledH) * 0.5;
        } else {
            clampedY = qBound(height() - scaledH, y, 0.0);
        }
    }

    if (qFuzzyCompare(m_panY, clampedY)) return;
    m_panY = clampedY;
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
    qreal factor = 1.3;
    qreal newZoom = qBound(0.05, m_zoom * factor, 32.0);
    if (qFuzzyCompare(newZoom, m_zoom)) return;

    qreal cx = width() * 0.5;
    qreal cy = height() * 0.5;
    qreal scaleRatio = newZoom / m_zoom;
    m_panX = cx - (cx - m_panX) * scaleRatio;
    m_panY = cy - (cy - m_panY) * scaleRatio;
    m_zoom = newZoom;

    qreal scaledW = WORLD_W * m_zoom;
    if (scaledW > 0) {
        while (m_panX > 0.0) m_panX -= scaledW;
        while (m_panX <= -scaledW) m_panX += scaledW;
    }
    setPanY(m_panY);

    emit zoomChanged();
    emit panChanged();
    update();
}

void MapCanvasItem::zoomOut()
{
    qreal factor = 1.0 / 1.3;
    qreal newZoom = qBound(0.05, m_zoom * factor, 32.0);
    if (qFuzzyCompare(newZoom, m_zoom)) return;

    qreal cx = width() * 0.5;
    qreal cy = height() * 0.5;
    qreal scaleRatio = newZoom / m_zoom;
    m_panX = cx - (cx - m_panX) * scaleRatio;
    m_panY = cy - (cy - m_panY) * scaleRatio;
    m_zoom = newZoom;

    qreal scaledW = WORLD_W * m_zoom;
    if (scaledW > 0) {
        while (m_panX > 0.0) m_panX -= scaledW;
        while (m_panX <= -scaledW) m_panX += scaledW;
    }
    setPanY(m_panY);

    emit zoomChanged();
    emit panChanged();
    update();
}

QPointF MapCanvasItem::screenToWorld(const QPointF& screenPos) const
{
    qreal scaledW = WORLD_W * m_zoom;
    if (scaledW <= 0) return QPointF(0, 0);

    qreal rawX = (screenPos.x() - m_panX) / m_zoom;
    qreal wx = std::fmod(rawX, (qreal)WORLD_W);
    if (wx < 0.0) wx += (qreal)WORLD_W;

    qreal wy = (screenPos.y() - m_panY) / m_zoom;
    return QPointF(wx, wy);
}

bool MapCanvasItem::isWorldPosValid(const QPointF& worldPos) const
{
    return worldPos.y() >= 0.0 && worldPos.y() < (qreal)WORLD_H;
}

QPointF MapCanvasItem::worldToScreen(const QPointF& worldPos) const
{
    return QPointF((worldPos.x() * m_zoom) + m_panX, (worldPos.y() * m_zoom) + m_panY);
}

void MapCanvasItem::paint(QPainter* painter)
{
    if (!m_bridge) return;
    const QImage& worldImg = m_bridge->worldImage();
    if (worldImg.isNull()) return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_zoom < 1.0);
    painter->setRenderHint(QPainter::Antialiasing, true);

    qreal scaledW = WORLD_W * m_zoom;
    qreal scaledH = WORLD_H * m_zoom;
    if (scaledW <= 0 || scaledH <= 0) return;

    QRectF viewport(0, 0, width(), height());

    // Continuous horizontal repeating tiles across the viewport
    int k_start = (int)std::floor((0.0 - m_panX) / scaledW);
    int k_end = (int)std::floor((width() - m_panX) / scaledW);

    for (int k = k_start; k <= k_end; ++k) {
        qreal tileLeft = m_panX + k * scaledW;
        QRectF targetRect(tileLeft, m_panY, scaledW, scaledH);
        QRectF visibleTarget = targetRect.intersected(viewport);
        if (visibleTarget.isEmpty()) continue;

        qreal sx = (visibleTarget.left() - tileLeft) / m_zoom;
        qreal sy = (visibleTarget.top() - m_panY) / m_zoom;
        qreal sw = visibleTarget.width() / m_zoom;
        qreal sh = visibleTarget.height() / m_zoom;

        sx = qBound(0.0, sx, (qreal)WORLD_W);
        sy = qBound(0.0, sy, (qreal)WORLD_H);
        sw = qBound(0.0, sw, (qreal)WORLD_W - sx);
        sh = qBound(0.0, sh, (qreal)WORLD_H - sy);

        QRectF visibleSource(sx, sy, sw, sh);
        painter->drawImage(visibleTarget, worldImg, visibleSource);
    }

    // Brush cursor preview ring: shows the exact brush radius on the map under cursor/touch
    if (m_isHovering && m_bridge &&
        (m_bridge->activeTool() == WorldEditorBridge::Brush || m_bridge->activeTool() == WorldEditorBridge::Eraser)) {
        qreal radiusScreen = m_bridge->brushRadius() * m_zoom;
        if (radiusScreen >= 1.0) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setBrush(Qt::NoBrush);

            QColor innerColor = (m_bridge->activeTool() == WorldEditorBridge::Eraser)
                ? QColor(239, 68, 68, 230)
                : ((m_bridge->activeMode() == WorldEditorBridge::Terrain) ? QColor(52, 211, 153, 230) : m_bridge->activeCountryColor());
            innerColor.setAlpha(220);

            // Outer dark contour for high contrast over light regions
            painter->setPen(QPen(QColor(15, 23, 42, 190), 2.5, Qt::SolidLine));
            painter->drawEllipse(m_currentScreenPos, radiusScreen, radiusScreen);

            // Inner colored accent ring
            painter->setPen(QPen(innerColor, 1.5, Qt::SolidLine));
            painter->drawEllipse(m_currentScreenPos, radiusScreen, radiusScreen);
            painter->restore();
        }
    }
}

void MapCanvasItem::onRegionDirty(int minX, int minY, int maxX, int maxY)
{
    Q_UNUSED(minX);
    Q_UNUSED(minY);
    Q_UNUSED(maxX);
    Q_UNUSED(maxY);
    update();
}

void MapCanvasItem::onWorldLoaded()
{
    update();
}

void MapCanvasItem::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = event->position();
    QPointF worldPosF = screenToWorld(m_lastMousePos);
    int wx = (int)std::floor(worldPosF.x());
    int wy = (int)std::floor(worldPosF.y());

    bool isHandTool = (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand);

    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && isHandTool)) {
        m_isPanning = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && m_bridge) {
        if (!isWorldPosValid(worldPosF)) {
            // Clicked outside vertical map bounds; ignore to avoid edge painting bug!
            event->accept();
            return;
        }

        int tool = m_bridge->activeTool();
        if (tool == WorldEditorBridge::Fill) {
            m_bridge->fillAt(wx, wy);
        } else if (tool == WorldEditorBridge::Picker) {
            m_bridge->pickAt(wx, wy);
        } else if (tool == WorldEditorBridge::Brush || tool == WorldEditorBridge::Eraser) {
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
    m_currentScreenPos = pos;
    m_isHovering = true;

    QPointF worldPosF = screenToWorld(pos);
    int wx = (int)std::floor(worldPosF.x());
    int wy = (int)std::floor(worldPosF.y());
    emit cursorWorldCoordsChanged(wx, wy);

    if (m_isPanning) {
        m_panX += delta.x();
        qreal scaledW = WORLD_W * m_zoom;
        if (scaledW > 0) {
            while (m_panX > 0.0) m_panX -= scaledW;
            while (m_panX <= -scaledW) m_panX += scaledW;
        }
        setPanY(m_panY + delta.y());
        emit panChanged();
        update();
        event->accept();
        return;
    }

    if (m_isPainting && m_bridge) {
        if (isWorldPosValid(worldPosF)) {
            if (isWorldPosValid(m_lastWorldPos)) {
                int dx = std::abs(wx - m_lastWorldPos.x());
                if (dx < WORLD_W / 2) {
                    strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx, wy);
                } else {
                    // Crossing the meridian seam
                    if (m_lastWorldPos.x() > wx) {
                        strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx + WORLD_W, wy);
                    } else {
                        strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx - WORLD_W, wy);
                    }
                }
            } else {
                m_bridge->paintAt(wx, wy);
            }
            m_lastWorldPos = QPoint(wx, wy);
        }
        event->accept();
    }
}

void MapCanvasItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand)) {
        m_isPanning = false;
        if (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
        event->accept();
    } else if (event->button() == Qt::LeftButton) {
        m_isPainting = false;
        event->accept();
    }
}

void MapCanvasItem::wheelEvent(QWheelEvent* event)
{
    QPointF mousePos = event->position();
    qreal angle = event->angleDelta().y();
    qreal factor = (angle > 0) ? 1.2 : 0.83333;

    qreal oldZoom = m_zoom;
    qreal newZoom = qBound(0.05, m_zoom * factor, 32.0);
    if (!qFuzzyCompare(newZoom, oldZoom)) {
        qreal scaleRatio = newZoom / oldZoom;
        m_panX = mousePos.x() - (mousePos.x() - m_panX) * scaleRatio;
        m_panY = mousePos.y() - (mousePos.y() - m_panY) * scaleRatio;
        m_zoom = newZoom;

        qreal scaledW = WORLD_W * m_zoom;
        if (scaledW > 0) {
            while (m_panX > 0.0) m_panX -= scaledW;
            while (m_panX <= -scaledW) m_panX += scaledW;
        }

        setPanY(m_panY);

        emit zoomChanged();
        emit panChanged();
        update();
    }

    event->accept();
}

void MapCanvasItem::touchEvent(QTouchEvent* event)
{
    const auto& points = event->points();
    if (points.isEmpty()) {
        QQuickPaintedItem::touchEvent(event);
        return;
    }

    if (points.count() >= 2) {
        // Multi-touch pinch & pan
        m_isPainting = false;
        m_isPanning = false;

        QPointF p0 = points[0].position();
        QPointF p1 = points[1].position();
        qreal dist = QLineF(p0, p1).length();
        QPointF mid = (p0 + p1) * 0.5;

        if (!m_pinchActive || event->type() == QEvent::TouchBegin) {
            m_pinchActive = true;
            m_initialPinchDist = dist;
            m_initialPinchZoom = m_zoom;
            m_lastPinchMid = mid;
        } else if (event->type() == QEvent::TouchUpdate) {
            if (m_initialPinchDist > 8.0 && dist > 8.0) {
                qreal factor = dist / m_initialPinchDist;
                qreal targetZoom = qBound(0.05, m_initialPinchZoom * factor, 32.0);

                // Zoom centered around the pinch midpoint
                if (!qFuzzyCompare(targetZoom, m_zoom)) {
                    qreal scaleRatio = targetZoom / m_zoom;
                    m_panX = mid.x() - (mid.x() - m_panX) * scaleRatio;
                    m_panY = mid.y() - (mid.y() - m_panY) * scaleRatio;
                    m_zoom = targetZoom;
                }

                // Two-finger pan translation
                QPointF deltaMid = mid - m_lastPinchMid;
                m_panX += deltaMid.x();
                m_panY += deltaMid.y();
                m_lastPinchMid = mid;

                qreal scaledW = WORLD_W * m_zoom;
                if (scaledW > 0) {
                    while (m_panX > 0.0) m_panX -= scaledW;
                    while (m_panX <= -scaledW) m_panX += scaledW;
                }
                setPanY(m_panY);

                emit zoomChanged();
                emit panChanged();
                update();
            }
        }
        if (event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel) {
            m_pinchActive = false;
        }
        event->accept();
        return;
    }

    // Single-finger touch handling (pan or paint / fill / pick)
    if (points.count() == 1) {
        QPointF pos = points[0].position();

        if (event->type() == QEvent::TouchBegin) {
            m_pinchActive = false;
            m_lastMousePos = pos;
            bool isHand = (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand);

            if (isHand) {
                m_isPanning = true;
                event->accept();
                return;
            }

            QPointF worldPosF = screenToWorld(pos);
            if (isWorldPosValid(worldPosF) && m_bridge) {
                int wx = (int)std::floor(worldPosF.x());
                int wy = (int)std::floor(worldPosF.y());
                int tool = m_bridge->activeTool();
                if (tool == WorldEditorBridge::Fill) {
                    m_bridge->fillAt(wx, wy);
                } else if (tool == WorldEditorBridge::Picker) {
                    m_bridge->pickAt(wx, wy);
                } else {
                    m_isPainting = true;
                    m_lastWorldPos = QPoint(wx, wy);
                    m_bridge->paintAt(wx, wy);
                }
                event->accept();
                return;
            }
        } else if (event->type() == QEvent::TouchUpdate) {
            QPointF delta = pos - m_lastMousePos;
            m_lastMousePos = pos;

            if (m_isPanning) {
                m_panX += delta.x();
                m_panY += delta.y();
                qreal scaledW = WORLD_W * m_zoom;
                if (scaledW > 0) {
                    while (m_panX > 0.0) m_panX -= scaledW;
                    while (m_panX <= -scaledW) m_panX += scaledW;
                }
                setPanY(m_panY);
                emit panChanged();
                update();
                event->accept();
                return;
            }

            QPointF worldPosF2 = screenToWorld(pos);
            if (isWorldPosValid(worldPosF2)) {
                int wx = (int)std::floor(worldPosF2.x());
                int wy = (int)std::floor(worldPosF2.y());
                emit cursorWorldCoordsChanged(wx, wy);
                if (m_isPainting && m_bridge) {
                    int dx = std::abs(wx - m_lastWorldPos.x());
                    if (dx < WORLD_W / 2) {
                        strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx, wy);
                    } else {
                        if (m_lastWorldPos.x() > wx) {
                            strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx + WORLD_W, wy);
                        } else {
                            strokeLine(m_lastWorldPos.x(), m_lastWorldPos.y(), wx - WORLD_W, wy);
                        }
                    }
                    m_lastWorldPos = QPoint(wx, wy);
                }
                event->accept();
                return;
            }
        } else if (event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel) {
            m_isPanning = false;
            m_isPainting = false;
            m_pinchActive = false;
            event->accept();
            return;
        }
    }

    QQuickPaintedItem::touchEvent(event);
}

void MapCanvasItem::hoverEnterEvent(QHoverEvent* event)
{
    m_isHovering = true;
    m_currentScreenPos = event->position();
    update();
}

void MapCanvasItem::hoverMoveEvent(QHoverEvent* event)
{
    m_isHovering = true;
    m_currentScreenPos = event->position();
    QPointF worldPosF = screenToWorld(m_currentScreenPos);
    emit cursorWorldCoordsChanged((int)std::floor(worldPosF.x()), (int)std::floor(worldPosF.y()));
    update();
}

void MapCanvasItem::hoverLeaveEvent(QHoverEvent* event)
{
    Q_UNUSED(event);
    m_isHovering = false;
    update();
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
            int wrappedX = ((currX % WORLD_W) + WORLD_W) % WORLD_W;
            m_bridge->paintAt(wrappedX, currY);
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
