#include "WorldEditorBridge.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

WorldEditorBridge::WorldEditorBridge(QObject* parent)
    : QObject(parent)
    , m_countryModel(new CountryListModel(this))
{
    // Auto-create initial Earth world by default
    newWorld("Tierra", true);
}

WorldEditorBridge::~WorldEditorBridge()
{
    if (m_world) {
        matalas_world_destroy(m_world);
        m_world = nullptr;
    }
}

void WorldEditorBridge::newWorld(const QString& name, bool earthPreset)
{
    if (m_world) {
        matalas_world_destroy(m_world);
        m_world = nullptr;
    }

    m_worldName = name;
    emit worldNameChanged();

    if (earthPreset) {
        // Find earth_terrain_8192x4096.png or earth_terrain_8k.bin.gz
        QString appDir = QCoreApplication::applicationDirPath();
        QString presetPath = "assets/maps/earth_terrain_8192x4096.png";

        // Try multiple paths
        if (!QFileInfo::exists(presetPath)) {
            presetPath = appDir + "/assets/maps/earth_terrain_8192x4096.png";
        }
        if (!QFileInfo::exists(presetPath)) {
            presetPath = "assets/presets/earth_terrain_8k.bin.gz";
        }

        QByteArray nameBytes = name.toUtf8();
        QByteArray pathBytes = presetPath.toUtf8();
        m_world = matalas_world_create_from_preset(nameBytes.constData(), pathBytes.constData());
    } else {
        QByteArray nameBytes = name.toUtf8();
        m_world = matalas_world_create_empty(nameBytes.constData());
    }

    // Create a starter country for convenience
    createCountry("Nuevo País", QColor(230, 81, 0)); // Warm Amber

    m_countryModel->syncFromWorld(m_world, m_activeCountryId);
    updateUndoState();
    emit worldLoaded();
    emit regionDirty(0, 0, 8191, 4095);
}

bool WorldEditorBridge::loadWorld(const QString& filePath)
{
    QByteArray pathBytes = filePath.toUtf8();
    WorldStateHandle loaded = matalas_world_load(pathBytes.constData());
    if (!loaded) {
        qWarning() << "Failed to load world from" << filePath;
        return false;
    }

    if (m_world) {
        matalas_world_destroy(m_world);
    }
    m_world = loaded;

    QFileInfo fi(filePath);
    m_worldName = fi.baseName();
    emit worldNameChanged();

    m_countryModel->syncFromWorld(m_world, m_activeCountryId);
    updateUndoState();
    emit worldLoaded();
    emit regionDirty(0, 0, 8191, 4095);
    return true;
}

bool WorldEditorBridge::saveWorld(const QString& filePath)
{
    if (!m_world) return false;
    QByteArray pathBytes = filePath.toUtf8();
    return matalas_world_save(m_world, pathBytes.constData());
}

void WorldEditorBridge::setActiveTool(int tool)
{
    if (m_activeTool == tool) return;
    m_activeTool = tool;
    if (m_world) {
        matalas_world_set_tool(m_world, tool);
    }
    emit activeToolChanged();
}

void WorldEditorBridge::setActiveMode(int mode)
{
    if (m_activeMode == mode) return;
    m_activeMode = mode;
    if (m_world) {
        matalas_world_set_mode(m_world, mode);
    }
    emit activeModeChanged();
}

void WorldEditorBridge::setBrushRadius(int radius)
{
    if (m_brushRadius == radius) return;
    m_brushRadius = radius;
    if (m_world) {
        matalas_world_set_brush_radius(m_world, radius);
    }
    emit brushRadiusChanged();
}

void WorldEditorBridge::setActiveCountryId(int id)
{
    if (m_activeCountryId == id) return;
    m_activeCountryId = id;
    if (m_world) {
        matalas_world_set_active_country(m_world, id);
    }
    m_countryModel->setSelectedCountry(id);
    emit activeCountryChanged();
}

int WorldEditorBridge::createCountry(const QString& name, const QColor& color)
{
    if (!m_world) return 0;
    QByteArray nameBytes = name.toUtf8();
    uint16_t id = matalas_world_create_country(
        m_world,
        nameBytes.constData(),
        color.red(),
        color.green(),
        color.blue(),
        color.alpha()
    );

    setActiveCountryId(id);
    m_countryModel->syncFromWorld(m_world, m_activeCountryId);
    return id;
}

bool WorldEditorBridge::updateCountry(int id, const QString& name, const QColor& color)
{
    if (!m_world) return false;
    QByteArray nameBytes = name.toUtf8();
    bool ok = matalas_world_update_country(
        m_world,
        id,
        nameBytes.constData(),
        color.red(),
        color.green(),
        color.blue(),
        color.alpha()
    );
    if (ok) {
        m_countryModel->syncFromWorld(m_world, m_activeCountryId);
        emit regionDirty(0, 0, 8191, 4095);
    }
    return ok;
}

bool WorldEditorBridge::deleteCountry(int id)
{
    if (!m_world) return false;
    FfiRect rect;
    bool ok = matalas_world_delete_country(m_world, id, &rect);
    if (ok) {
        m_countryModel->syncFromWorld(m_world, m_activeCountryId);
        emit regionDirty(rect.min_x, rect.min_y, rect.max_x, rect.max_y);
    }
    return ok;
}

bool WorldEditorBridge::paintAt(int x, int y)
{
    if (!m_world) return false;
    FfiRect dirty;
    bool ok = matalas_world_paint_at(m_world, x, y, &dirty);
    if (ok) {
        emit regionDirty(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
        updateUndoState();
    }
    return ok;
}

bool WorldEditorBridge::fillAt(int x, int y)
{
    if (!m_world) return false;
    FfiRect dirty;
    bool ok = matalas_world_fill_at(m_world, x, y, &dirty);
    if (ok) {
        emit regionDirty(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
        updateUndoState();
    }
    return ok;
}

int WorldEditorBridge::pickAt(int x, int y)
{
    if (!m_world) return 0;
    uint16_t id = matalas_world_pick_at(m_world, x, y);
    if (id > 0) {
        setActiveCountryId(id);
    }
    return id;
}

bool WorldEditorBridge::undo()
{
    if (!m_world) return false;
    FfiRect dirty;
    uint32_t mode = 0;
    bool ok = matalas_world_undo(m_world, &dirty, &mode);
    if (ok) {
        emit regionDirty(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
        updateUndoState();
    }
    return ok;
}

bool WorldEditorBridge::redo()
{
    if (!m_world) return false;
    FfiRect dirty;
    uint32_t mode = 0;
    bool ok = matalas_world_redo(m_world, &dirty, &mode);
    if (ok) {
        emit regionDirty(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
        updateUndoState();
    }
    return ok;
}

bool WorldEditorBridge::canUndo() const
{
    if (!m_world) return false;
    return matalas_world_can_undo(m_world);
}

bool WorldEditorBridge::canRedo() const
{
    if (!m_world) return false;
    return matalas_world_can_redo(m_world);
}

void WorldEditorBridge::updateUndoState()
{
    emit canUndoChanged();
    emit canRedoChanged();
}
