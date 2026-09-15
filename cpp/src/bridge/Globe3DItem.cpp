#include "Globe3DItem.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTouchEvent>
#include <QLineF>
#include <QCursor>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QRadialGradient>
#include <cmath>

static const int WORLD_W = 8192;
static const int WORLD_H = 4096;
static const qreal PI = 3.14159265358979323846;

Globe3DItem::Globe3DItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptTouchEvents(true);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
}

void Globe3DItem::setBridge(WorldEditorBridge* bridge)
{
    if (m_bridge == bridge) return;

    if (m_bridge) {
        disconnect(m_bridge, nullptr, this, nullptr);
    }

    m_bridge = bridge;

    if (m_bridge) {
        connect(m_bridge, &WorldEditorBridge::regionDirty, this, &Globe3DItem::onRegionDirty);
        connect(m_bridge, &WorldEditorBridge::worldLoaded, this, &Globe3DItem::onWorldLoaded);
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

void Globe3DItem::setYaw(qreal y)
{
    while (y > PI) y -= 2.0 * PI;
    while (y < -PI) y += 2.0 * PI;
    if (qFuzzyCompare(m_yaw, y)) return;
    m_yaw = y;
    emit yawChanged();
    update();
}

void Globe3DItem::setPitch(qreal p)
{
    qreal clamped = qBound(-1.45, p, 1.45);
    if (qFuzzyCompare(m_pitch, clamped)) return;
    m_pitch = clamped;
    emit pitchChanged();
    update();
}

void Globe3DItem::setZoom(qreal z)
{
    qreal clamped = qBound(0.4, z, 5.0);
    if (qFuzzyCompare(m_zoom, clamped)) return;
    m_zoom = clamped;
    emit zoomChanged();
    update();
}

void Globe3DItem::resetView()
{
    m_yaw = 0.0;
    m_pitch = 0.2;
    m_zoom = 1.0;
    emit yawChanged();
    emit pitchChanged();
    emit zoomChanged();
    update();
}

void Globe3DItem::zoomIn()
{
    setZoom(m_zoom * 1.2);
}

void Globe3DItem::zoomOut()
{
    setZoom(m_zoom / 1.2);
}

void Globe3DItem::onRegionDirty(int minX, int minY, int maxX, int maxY)
{
    Q_UNUSED(minX);
    Q_UNUSED(minY);
    Q_UNUSED(maxX);
    Q_UNUSED(maxY);
    update();
}

void Globe3DItem::onWorldLoaded()
{
    update();
}

bool Globe3DItem::raycastSphere(const QPointF& screenPos, int& outWx, int& outWy) const
{
    if (width() <= 0 || height() <= 0) return false;

    qreal cx = width() * 0.5;
    qreal cy = height() * 0.5;
    qreal R = qMin(width(), height()) * 0.44 * m_zoom;
    if (R <= 1.0) return false;

    qreal dx = (screenPos.x() - cx) / R;
    qreal dy = (screenPos.y() - cy) / R;
    qreal distSq = dx * dx + dy * dy;

    if (distSq > 1.0) {
        return false;
    }

    qreal dz = std::sqrt(qMax(0.0, 1.0 - distSq));

    // Eye-space normal vector
    qreal vx = dx;
    qreal vy = -dy;
    qreal vz = dz;

    // Inverse camera rotation (Pitch around X, then Yaw around Y)
    qreal cosP = std::cos(m_pitch);
    qreal sinP = std::sin(m_pitch);
    qreal py1 = vy * cosP - vz * sinP;
    qreal pz1 = vy * sinP + vz * cosP;

    qreal cosY = std::cos(m_yaw);
    qreal sinY = std::sin(m_yaw);
    qreal nx = vx * cosY + pz1 * sinY;
    qreal ny = py1;
    qreal nz = -vx * sinY + pz1 * cosY;

    qreal lat = std::asin(qBound(-1.0, ny, 1.0));
    qreal lon = std::atan2(nx, nz);

    qreal normX = (lon + PI) / (2.0 * PI);
    qreal normY = (PI * 0.5 - lat) / PI;

    outWx = qBound(0, (int)std::floor(normX * WORLD_W), WORLD_W - 1);
    outWy = qBound(0, (int)std::floor(normY * WORLD_H), WORLD_H - 1);
    return true;
}

void Globe3DItem::paint(QPainter* painter)
{
    int w = (int)width();
    int h = (int)height();
    if (w <= 0 || h <= 0) return;

    qreal cx = width() * 0.5;
    qreal cy = height() * 0.5;
    qreal R = qMin(width(), height()) * 0.44 * m_zoom;
    if (R <= 2.0) return;

    QRectF sphereRect(cx - R, cy - R, 2.0 * R, 2.0 * R);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

    // Atmospheric deep glow behind the globe
    QRadialGradient glow(QPointF(cx, cy), R * 1.15);
    glow.setColorAt(0.75, QColor(30, 58, 138, 40));
    glow.setColorAt(0.95, QColor(56, 189, 248, 25));
    glow.setColorAt(1.0, Qt::transparent);
    painter->fillRect(QRectF(cx - R * 1.15, cy - R * 1.15, 2.3 * R, 2.3 * R), glow);

    // Viewport-bounded rendering:
    // Scale down 2x during active interaction (rotation/pinch) or if viewport is large,
    // and ONLY compute pixels strictly inside the visible screen bounds [0..w, 0..h].
    // This completely eliminates quadratic zoom-in lag when zoomed in!
    bool isInteracting = m_isRotating || m_pinchActive;
    int scale = isInteracting ? 2 : 1;

    int bufW = qMax(1, w / scale);
    int bufH = qMax(1, h / scale);

    QImage globeImage(bufW, bufH, QImage::Format_ARGB32_Premultiplied);
    globeImage.fill(Qt::transparent);

    qreal bcx = cx / scale;
    qreal bcy = cy / scale;
    qreal bR = R / scale;
    qreal bR2 = bR * bR;
    qreal invBR = 1.0 / bR;

    qreal cosP = std::cos(m_pitch);
    qreal sinP = std::sin(m_pitch);
    qreal cosY = std::cos(m_yaw);
    qreal sinY = std::sin(m_yaw);

    // Sun light vector in eye space
    static const qreal lx = 0.38;
    static const qreal ly = 0.35;
    static const qreal lz = 0.85;

    // Bounded strictly to the intersection of the sphere circle and visible buffer
    int minY = qBound(0, (int)std::floor(bcy - bR - 1.0), bufH - 1);
    int maxY = qBound(0, (int)std::ceil(bcy + bR + 1.0), bufH - 1);

    if (!m_bridge) return;
    const QImage& worldImg = m_bridge->worldImage();
    if (worldImg.isNull()) return;
    const uchar* worldData = worldImg.constBits();

    for (int sy = minY; sy <= maxY; ++sy) {
        qreal dy = (sy - bcy);
        qreal dy2 = dy * dy;
        if (dy2 > (bR + 1.2) * (bR + 1.2)) continue;

        qreal maxDx = std::sqrt(qMax(0.0, (bR + 1.2) * (bR + 1.2) - dy2));
        int minX = qBound(0, (int)std::floor(bcx - maxDx), bufW - 1);
        int maxX = qBound(0, (int)std::ceil(bcx + maxDx), bufW - 1);

        QRgb* scanline = reinterpret_cast<QRgb*>(globeImage.scanLine(sy));

        qreal vy = -dy * invBR;
        qreal vy_cosP = vy * cosP;
        qreal vy_sinP = vy * sinP;
        qreal vy_ly = vy * ly;

        for (int sx = minX; sx <= maxX; ++sx) {
            qreal dx = (sx - bcx);
            qreal distSq = dx * dx + dy2;
            if (distSq > (bR + 0.8) * (bR + 0.8)) continue;

            qreal dist = std::sqrt(distSq);
            qreal edgeAlpha = (dist > bR - 1.0) ? qBound(0.0, (bR + 0.8 - dist) * 0.9, 1.0) : 1.0;

            qreal dz = std::sqrt(qMax(0.0, 1.0 - (distSq / bR2)));
            qreal vx = dx * invBR;
            qreal vz = dz;

            qreal py1 = vy_cosP - vz * sinP;
            qreal pz1 = vy_sinP + vz * cosP;

            qreal nx = vx * cosY + pz1 * sinY;
            qreal ny = py1;
            qreal nz = -vx * sinY + pz1 * cosY;

            qreal lat = std::asin(qBound(-1.0, ny, 1.0));
            qreal lon = std::atan2(nx, nz);

            int wx = qBound(0, (int)std::floor(((lon + PI) / (2.0 * PI)) * WORLD_W), WORLD_W - 1);
            int wy = qBound(0, (int)std::floor(((PI * 0.5 - lat) / PI) * WORLD_H), WORLD_H - 1);

            int srcIdx = (wy * WORLD_W + wx) * 4;
            uchar r = worldData[srcIdx];
            uchar g = worldData[srcIdx + 1];
            uchar b = worldData[srcIdx + 2];

            // 3D Lighting & atmosphere
            qreal dotL = vx * lx + vy_ly + vz * lz;
            qreal diffuse = qMax(0.0, dotL) * 0.42 + 0.58;
            qreal rim = std::pow(1.0 - dz, 2.5) * 0.38;

            // Subtle specular water shine
            qreal spec = 0.0;
            if (b > r + 15 && b > g) {
                spec = std::pow(qMax(0.0, dotL), 12.0) * 0.25;
            }

            int finalR = qBound(0, (int)((r * diffuse + rim * 80 + spec * 220) * edgeAlpha), 255);
            int finalG = qBound(0, (int)((g * diffuse + rim * 150 + spec * 240) * edgeAlpha), 255);
            int finalB = qBound(0, (int)((b * diffuse + rim * 255 + spec * 255) * edgeAlpha), 255);
            int finalA = qBound(0, (int)(255 * edgeAlpha), 255);

            scanline[sx] = qRgba(finalR, finalG, finalB, finalA);
        }
    }

    // Draw the viewport buffer directly onto the widget with nearest-neighbor crispness
    painter->drawImage(QRectF(0, 0, w, h), globeImage);

    // Crisp atmospheric rim contour (if sphere rim is on screen)
    if (R < qMax(w, h) * 1.5) {
        painter->setPen(QPen(QColor(147, 197, 253, 70), 1.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(sphereRect);
    }
}

void Globe3DItem::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = event->position();
    int wx = 0, wy = 0;
    bool hit = raycastSphere(m_lastMousePos, wx, wy);

    bool isHand = (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand);

    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton || !hit || isHand) {
        m_isRotating = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && hit && m_bridge) {
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
    }
}

void Globe3DItem::mouseMoveEvent(QMouseEvent* event)
{
    QPointF pos = event->position();
    QPointF delta = pos - m_lastMousePos;
    m_lastMousePos = pos;

    if (m_isRotating) {
        setYaw(m_yaw - delta.x() * 0.007);
        setPitch(m_pitch - delta.y() * 0.007);
        event->accept();
        return;
    }

    int wx = 0, wy = 0;
    if (raycastSphere(pos, wx, wy)) {
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
            event->accept();
        }
    }
}

void Globe3DItem::strokeLine(int x0, int y0, int x1, int y1)
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

void Globe3DItem::hoverMoveEvent(QHoverEvent* event)
{
    int wx = 0, wy = 0;
    if (raycastSphere(event->position(), wx, wy)) {
        emit cursorWorldCoordsChanged(wx, wy);
    }
}

void Globe3DItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isRotating) {
        m_isRotating = false;
        if (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
        update(); // High-quality re-render upon release
        event->accept();
    } else if (event->button() == Qt::LeftButton) {
        m_isPainting = false;
        event->accept();
    }
}

void Globe3DItem::wheelEvent(QWheelEvent* event)
{
    qreal delta = event->angleDelta().y();
    if (delta > 0) {
        setZoom(m_zoom * 1.15);
    } else {
        setZoom(m_zoom / 1.15);
    }
    event->accept();
}

void Globe3DItem::touchEvent(QTouchEvent* event)
{
    const auto& points = event->points();
    if (points.isEmpty()) {
        QQuickPaintedItem::touchEvent(event);
        return;
    }

    if (points.count() >= 2) {
        m_isPainting = false;
        m_isRotating = false;

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
                setZoom(qBound(0.4, m_initialPinchZoom * factor, 5.0));

                QPointF deltaMid = mid - m_lastPinchMid;
                setYaw(m_yaw - deltaMid.x() * 0.007);
                setPitch(m_pitch - deltaMid.y() * 0.007);
                m_lastPinchMid = mid;
            }
        }
        if (event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel) {
            m_pinchActive = false;
            update(); // High-quality re-render upon release
        }
        event->accept();
        return;
    }

    // Single-finger touch handling (drag rotation, painting, fill, picker)
    if (points.count() == 1) {
        QPointF pos = points[0].position();

        if (event->type() == QEvent::TouchBegin) {
            m_pinchActive = false;
            m_lastMousePos = pos;
            int wx = 0, wy = 0;
            bool hit = raycastSphere(pos, wx, wy);
            bool isHand = (m_bridge && m_bridge->activeTool() == WorldEditorBridge::Hand);

            if (!hit || isHand) {
                m_isRotating = true;
                event->accept();
                return;
            }

            if (hit && m_bridge) {
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

            if (m_isRotating) {
                setYaw(m_yaw - delta.x() * 0.007);
                setPitch(m_pitch - delta.y() * 0.007);
                event->accept();
                return;
            }

            int wx = 0, wy = 0;
            if (raycastSphere(pos, wx, wy)) {
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
            if (m_isRotating) {
                m_isRotating = false;
                update();
            }
            m_isPainting = false;
            m_pinchActive = false;
            event->accept();
            return;
        }
    }

    QQuickPaintedItem::touchEvent(event);
}
