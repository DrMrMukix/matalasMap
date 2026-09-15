#include "WorldEditorBridge.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QSvgRenderer>
#include <QPainter>
#include <QImage>
#include <QTimer>

WorldEditorBridge::WorldEditorBridge(QObject* parent)
    : QObject(parent)
    , m_countryModel(new CountryListModel(this))
    , m_masterWorldImage(8192, 4096, QImage::Format_RGBA8888)
{
    m_masterWorldImage.fill(QColor(36, 79, 117));
    // Auto-create initial Earth world by default
    newWorld("Tierra", 1);
}

void WorldEditorBridge::renderRegionToMaster(int minX, int minY, int maxX, int maxY)
{
    if (!m_world || m_masterWorldImage.isNull()) return;

    minX = qBound(0, minX, 8191);
    minY = qBound(0, minY, 4095);
    maxX = qBound(0, maxX, 8191);
    maxY = qBound(0, maxY, 4095);

    int w = maxX - minX + 1;
    int h = maxY - minY + 1;
    if (w <= 0 || h <= 0) return;

    uchar* destPtr = m_masterWorldImage.scanLine(minY) + (minX * 4);
    size_t stride = m_masterWorldImage.bytesPerLine();
    size_t maxLen = (size_t)(m_masterWorldImage.sizeInBytes() - (destPtr - m_masterWorldImage.bits()));

    matalas_world_render_rect_strided(
        m_world,
        (uint32_t)minX, (uint32_t)minY, (uint32_t)maxX, (uint32_t)maxY,
        destPtr,
        maxLen,
        stride
    );
}

WorldEditorBridge::~WorldEditorBridge()
{
    if (m_world) {
        matalas_world_destroy(m_world);
        m_world = nullptr;
    }
}

void WorldEditorBridge::newWorld(const QString& name, int presetType)
{
    if (m_world) {
        matalas_world_destroy(m_world);
        m_world = nullptr;
    }

    m_worldName = name;
    emit worldNameChanged();

    QString appDir = QCoreApplication::applicationDirPath();

    if (presetType > 0) {
        QStringList candidates = {
            QDir(appDir).filePath("assets/maps/earth_terrain_8192x4096.bin.gz"),
            QDir(appDir).filePath("assets/maps/earth_terrain_8192x4096.png"),
            QDir(appDir).filePath("../assets/maps/earth_terrain_8192x4096.bin.gz"),
            QDir(appDir).filePath("../assets/maps/earth_terrain_8192x4096.png"),
            "assets/maps/earth_terrain_8192x4096.bin.gz",
            "assets/maps/earth_terrain_8192x4096.png",
            "../assets/maps/earth_terrain_8192x4096.bin.gz",
            "../assets/maps/earth_terrain_8192x4096.png",
            "assets:/assets/maps/earth_terrain_8192x4096.bin.gz",
            "assets:/assets/maps/earth_terrain_8192x4096.png"
        };
        QByteArray fileData;
        QString presetPath;
        for (const QString& cand : candidates) {
            if (QFileInfo::exists(cand)) {
                presetPath = cand;
                QFile f(cand);
                if (f.open(QIODevice::ReadOnly)) {
                    fileData = f.readAll();
                    if (!fileData.isEmpty()) break;
                }
            }
        }

        QByteArray nameBytes = name.toUtf8();
        if (!fileData.isEmpty()) {
            m_world = matalas_world_create_with_terrain_buffer(
                nameBytes.constData(),
                reinterpret_cast<const uint8_t*>(fileData.constData()),
                fileData.size()
            );
        } else {
            QByteArray pathBytes = presetPath.toUtf8();
            m_world = matalas_world_create_from_preset(nameBytes.constData(), pathBytes.constData());
        }
    } else {
        QByteArray nameBytes = name.toUtf8();
        m_world = matalas_world_create_empty(nameBytes.constData());
    }

    // If presetType == 2 (Tierra con naciones 2026), try loading pre-rasterized 2026 world with 200+ sovereign nations
    if (presetType == 2) {
        QStringList matalasCandidates = {
            QDir(appDir).filePath("assets/presets/Tierra_con_Naciones_2026.matalas"),
            QDir(appDir).filePath("../assets/presets/Tierra_con_Naciones_2026.matalas"),
            "assets/presets/Tierra_con_Naciones_2026.matalas",
            "../assets/presets/Tierra_con_Naciones_2026.matalas",
            "assets:/assets/presets/Tierra_con_Naciones_2026.matalas"
        };
        WorldStateHandle loaded = nullptr;
        for (const QString& cand : matalasCandidates) {
            if (QFileInfo::exists(cand)) {
                QFile f(cand);
                if (f.open(QIODevice::ReadOnly)) {
                    QByteArray data = f.readAll();
                    if (!data.isEmpty()) {
                        loaded = matalas_world_load_from_memory(
                            reinterpret_cast<const uint8_t*>(data.constData()),
                            data.size()
                        );
                        if (loaded) break;
                    }
                }
            }
        }
        if (loaded) {
            if (m_world) {
                matalas_world_destroy(m_world);
            }
            m_world = loaded;
            m_activeMode = Political;
            emit activeModeChanged();
            m_countryModel->syncFromWorld(m_world, m_activeCountryId);
            updateUndoState();
            renderRegionToMaster(0, 0, 8191, 4095);
            emit worldLoaded();
            emit regionDirty(0, 0, 8191, 4095);
            return;
        }

        uint16_t idEsp = createCountry("España", QColor(198, 40, 40));
        setCountryFlag(idEsp, "assets/flags/esp_1978.svg");

        uint16_t idFra = createCountry("Francia", QColor(30, 136, 229));
        setCountryFlag(idFra, "assets/flags/fra_1958.svg");

        uint16_t idDeu = createCountry("Alemania", QColor(255, 179, 0));
        setCountryFlag(idDeu, "assets/flags/deu_1990.svg");

        uint16_t idGbr = createCountry("Reino Unido", QColor(40, 53, 147));
        setCountryFlag(idGbr, "assets/flags/gbr_1927.svg");

        uint16_t idIta = createCountry("Italia", QColor(46, 125, 50));
        setCountryFlag(idIta, "assets/flags/ita_1946.svg");

        uint16_t idUsa = createCountry("Estados Unidos", QColor(21, 101, 192));
        setCountryFlag(idUsa, "assets/flags/usa_1960.svg");

        uint16_t idMex = createCountry("México", QColor(0, 137, 123));
        setCountryFlag(idMex, "assets/flags/mex_1824.svg");

        uint16_t idBra = createCountry("Brasil", QColor(67, 160, 71));
        setCountryFlag(idBra, "assets/flags/bra_1889.svg");

        uint16_t idArg = createCountry("Argentina", QColor(79, 195, 247));
        setCountryFlag(idArg, "assets/flags/arg_1816.svg");

        uint16_t idChl = createCountry("Chile", QColor(229, 57, 53));
        setCountryFlag(idChl, "assets/flags/chl_1817.svg");

        uint16_t idJpn = createCountry("Japón", QColor(239, 83, 80));
        setCountryFlag(idJpn, "assets/flags/jpn_1947.svg");

        uint16_t idChn = createCountry("China", QColor(216, 27, 96));
        setCountryFlag(idChn, "assets/flags/chn_1949.svg");
    } else {
        createCountry("Nuevo País", QColor(230, 81, 0));
    }

    m_countryModel->syncFromWorld(m_world, m_activeCountryId);
    for (int i = 0; i < m_countryModel->rowCount(); ++i) {
        QModelIndex idx = m_countryModel->index(i, 0);
        int cid = m_countryModel->data(idx, CountryListModel::IdRole).toInt();
        QString flag = m_countryModel->data(idx, CountryListModel::FlagPathRole).toString();
        if (!flag.isEmpty()) {
            rasterizeAndSendFlagToRust(cid, flag);
        }
    }
    updateUndoState();
    renderRegionToMaster(0, 0, 8191, 4095);
    emit worldLoaded();
    emit regionDirty(0, 0, 8191, 4095);
}

bool WorldEditorBridge::loadWorld(const QString& filePath)
{
    WorldStateHandle loaded = nullptr;
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray data = f.readAll();
        if (!data.isEmpty()) {
            loaded = matalas_world_load_from_memory(
                reinterpret_cast<const uint8_t*>(data.constData()),
                data.size()
            );
        }
    }
    if (!loaded) {
        QByteArray pathBytes = filePath.toUtf8();
        loaded = matalas_world_load(pathBytes.constData());
    }
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
    for (int i = 0; i < m_countryModel->rowCount(); ++i) {
        QModelIndex idx = m_countryModel->index(i, 0);
        int cid = m_countryModel->data(idx, CountryListModel::IdRole).toInt();
        QString flag = m_countryModel->data(idx, CountryListModel::FlagPathRole).toString();
        if (!flag.isEmpty()) {
            rasterizeAndSendFlagToRust(cid, flag);
        }
    }
    updateUndoState();
    renderRegionToMaster(0, 0, 8191, 4095);
    emit worldLoaded();
    emit regionDirty(0, 0, 8191, 4095);
    return true;
}

QString WorldEditorBridge::activeCountryName() const
{
    if (!m_countryModel) return "Elegir País";
    for (int i = 0; i < m_countryModel->rowCount(); ++i) {
        QModelIndex idx = m_countryModel->index(i, 0);
        if (m_countryModel->data(idx, CountryListModel::IdRole).toInt() == m_activeCountryId) {
            return m_countryModel->data(idx, CountryListModel::NameRole).toString();
        }
    }
    return "Elegir País";
}

QColor WorldEditorBridge::activeCountryColor() const
{
    if (!m_countryModel) return QColor(255, 87, 34);
    for (int i = 0; i < m_countryModel->rowCount(); ++i) {
        QModelIndex idx = m_countryModel->index(i, 0);
        if (m_countryModel->data(idx, CountryListModel::IdRole).toInt() == m_activeCountryId) {
            return m_countryModel->data(idx, CountryListModel::ColorRole).value<QColor>();
        }
    }
    return QColor(255, 87, 34);
}

QString WorldEditorBridge::activeCountryFlag() const
{
    if (!m_countryModel) return "";
    for (int i = 0; i < m_countryModel->rowCount(); ++i) {
        QModelIndex idx = m_countryModel->index(i, 0);
        if (m_countryModel->data(idx, CountryListModel::IdRole).toInt() == m_activeCountryId) {
            return m_countryModel->data(idx, CountryListModel::FlagPathRole).toString();
        }
    }
    return "";
}

QVector<FfiFlagAnchor> WorldEditorBridge::getFlagAnchors() const
{
    QVector<FfiFlagAnchor> anchors;
    if (!m_world) return anchors;

    FfiFlagAnchor buf[256];
    size_t count = matalas_world_get_flag_anchors(m_world, buf, 256);
    anchors.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        anchors.append(buf[i]);
    }
    return anchors;
}

QString WorldEditorBridge::getCountryFlagPath(int id) const
{
    if (!m_countryModel) return "";
    for (int i = 0; i < m_countryModel->rowCount(); ++i) {
        QModelIndex idx = m_countryModel->index(i, 0);
        if (m_countryModel->data(idx, CountryListModel::IdRole).toInt() == id) {
            return m_countryModel->data(idx, CountryListModel::FlagPathRole).toString();
        }
    }
    return "";
}

QString WorldEditorBridge::getDefaultSavePath() const
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) {
        baseDir = QCoreApplication::applicationDirPath();
    }
    QDir dir(baseDir);
    dir.mkpath("saves");
    QString safeName = m_worldName.isEmpty() ? "Mi_Mundo" : m_worldName;
    safeName.replace(QRegularExpression("[^a-zA-Z0-9_\\-]"), "_");
    return dir.filePath("saves/" + safeName + ".matalas");
}

bool WorldEditorBridge::saveWorld(const QString& filePath)
{
    if (!m_world) return false;

    QString finalPath = filePath;
    if (finalPath.isEmpty()) {
        finalPath = getDefaultSavePath();
    } else if (QFileInfo(finalPath).isRelative()) {
        QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (baseDir.isEmpty()) baseDir = QCoreApplication::applicationDirPath();
        finalPath = QDir(baseDir).filePath(finalPath);
    }

    QFileInfo fi(finalPath);
    QDir dir = fi.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QByteArray pathBytes = finalPath.toUtf8();
    return matalas_world_save(m_world, pathBytes.constData());
}

bool WorldEditorBridge::saveWorldAs(const QString& filePath, const QString& newName)
{
    m_worldName = newName;
    emit worldNameChanged();
    return saveWorld(filePath);
}

void WorldEditorBridge::setIsLoading(bool loading, const QString& msg)
{
    if (!msg.isEmpty() && m_loadingMessage != msg) {
        m_loadingMessage = msg;
        emit loadingMessageChanged();
    }
    if (m_isLoading != loading) {
        m_isLoading = loading;
        emit isLoadingChanged();
    }
}

QString WorldEditorBridge::getSavePathForName(const QString& name) const
{
    QString safeName = name.trimmed();
    if (safeName.isEmpty()) safeName = "Mi_Mundo";
    safeName.replace(QRegularExpression("[^a-zA-Z0-9_\\-\\s]"), "");
    safeName.replace(" ", "_");

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) {
        baseDir = QCoreApplication::applicationDirPath();
    }
    QDir dir(baseDir);
    dir.mkpath("saves");
    return dir.filePath("saves/" + safeName + ".matalas");
}

bool WorldEditorBridge::checkSaveExists(const QString& name) const
{
    QString path = getSavePathForName(name);
    return QFile::exists(path);
}

bool WorldEditorBridge::saveWorldNamed(const QString& name, bool overwrite)
{
    QString targetPath = getSavePathForName(name);
    if (QFile::exists(targetPath) && !overwrite) {
        return false;
    }
    return saveWorldAs(targetPath, name);
}

void WorldEditorBridge::startAsyncNewWorld(const QString& name, int presetType)
{
    QString msg = "Generando mapa en blanco...";
    if (presetType == 1) msg = "Cargando mapa de la Tierra 8K...";
    else if (presetType == 2) msg = "Cargando Tierra con Naciones 2026...";

    setIsLoading(true, msg);
    QTimer::singleShot(60, this, [this, name, presetType]() {
        newWorld(name, presetType);
        setIsLoading(false);
    });
}

void WorldEditorBridge::startAsyncLoadWorld(const QString& filePath)
{
    setIsLoading(true, "Cargando partida guardada...");
    QTimer::singleShot(60, this, [this, filePath]() {
        loadWorld(filePath);
        setIsLoading(false);
    });
}

bool WorldEditorBridge::deleteSavedWorld(const QString& filePath)
{
    QFile f(filePath);
    if (f.exists()) {
        return f.remove();
    }
    return false;
}

QVariantList WorldEditorBridge::getSavedWorldsList()
{
    QVariantList list;
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString appDir = QCoreApplication::applicationDirPath();

    QStringList searchDirs = {
        QDir(appDataDir).filePath("saves"),
        QDir(appDir).filePath("saves"),
        QDir(appDir).filePath("assets/presets"),
        "saves",
        "assets/presets"
    };

    QSet<QString> seenPaths;

    for (const QString& dirPath : searchDirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        QFileInfoList entries = dir.entryInfoList(QStringList() << "*.matalas", QDir::Files, QDir::Time);
        for (const QFileInfo& fi : entries) {
            QString absPath = fi.absoluteFilePath();
            if (seenPaths.contains(absPath)) continue;
            seenPaths.insert(absPath);

            QVariantMap item;
            item["name"] = fi.baseName();
            item["path"] = absPath;
            item["filePath"] = absPath;
            item["modified"] = fi.lastModified().toString("yyyy-MM-dd hh:mm");
            item["date"] = item["modified"];
            item["sizeBytes"] = (qulonglong)fi.size();
            item["size"] = QString("%1 KB").arg(qMax(1LL, fi.size() / 1024));
            list.append(item);
        }
    }
    return list;
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

void WorldEditorBridge::setDisplayMode(int mode)
{
    if (m_displayMode == mode) return;
    m_displayMode = mode;
    if (m_world) {
        matalas_world_set_display_mode(m_world, mode);
    }
    if (m_displayMode == 1 && m_countryModel) {
        for (int i = 0; i < m_countryModel->rowCount(); ++i) {
            QModelIndex idx = m_countryModel->index(i, 0);
            int cid = m_countryModel->data(idx, CountryListModel::IdRole).toInt();
            QString flag = m_countryModel->data(idx, CountryListModel::FlagPathRole).toString();
            if (!flag.isEmpty()) {
                rasterizeAndSendFlagToRust(cid, flag);
            }
        }
    }
    renderRegionToMaster(0, 0, 8191, 4095);
    emit displayModeChanged();
    emit regionDirty(0, 0, 8191, 4095);
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

void WorldEditorBridge::setCountryFlag(int id, const QString& flagPath)
{
    if (!m_world) return;
    QByteArray pathBytes = flagPath.toUtf8();
    matalas_world_set_country_flag(m_world, id, pathBytes.constData());
    rasterizeAndSendFlagToRust(id, flagPath);
    m_countryModel->syncFromWorld(m_world, m_activeCountryId);
    if (m_displayMode == 1) {
        renderRegionToMaster(0, 0, 8191, 4095);
        emit regionDirty(0, 0, 8191, 4095);
    }
}

void WorldEditorBridge::rasterizeAndSendFlagToRust(int id, const QString& flagPath)
{
    if (!m_world || flagPath.isEmpty()) return;

    QString cleanPath = flagPath;
    if (cleanPath.startsWith("file:///")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }

    QString stripped = cleanPath.startsWith("assets/") ? cleanPath.mid(7) : cleanPath;

    QStringList candidates = {
        cleanPath,
        "assets:/" + cleanPath,
        "assets:/assets/" + stripped,
        "assets:/" + stripped,
        QDir(QCoreApplication::applicationDirPath()).filePath(cleanPath),
        QDir(QCoreApplication::applicationDirPath() + "/assets").filePath(stripped),
        QDir(QCoreApplication::applicationDirPath() + "/../../..").filePath(cleanPath),
        QDir(QCoreApplication::applicationDirPath() + "/../..").filePath(cleanPath),
        QDir::current().filePath(cleanPath),
        "C:/Users/marti/Documents/matalasMap/" + cleanPath
    };

    QByteArray data;
    bool found = false;
    for (const QString& candidate : candidates) {
        QFile f(candidate);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            data = f.readAll();
            f.close();
            if (!data.isEmpty()) {
                found = true;
                break;
            }
        }
    }

    if (!found || data.isEmpty()) return;

    QImage img(256, 170, QImage::Format_RGBA8888);
    img.fill(Qt::transparent);

    if (cleanPath.endsWith(".svg", Qt::CaseInsensitive)) {
        QSvgRenderer renderer(data);
        if (renderer.isValid()) {
            QPainter painter(&img);
            renderer.render(&painter, QRectF(0, 0, 256, 170));
        }
    } else {
        QImage loaded;
        if (loaded.loadFromData(data)) {
            img = loaded.scaled(256, 170, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGBA8888);
        }
    }

    if (!img.isNull()) {
        matalas_world_set_country_flag_rgba(
            m_world,
            (uint16_t)id,
            img.width(),
            img.height(),
            img.constBits(),
            img.sizeInBytes()
        );
    }
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
        renderRegionToMaster(0, 0, 8191, 4095);
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
        renderRegionToMaster(rect.min_x, rect.min_y, rect.max_x, rect.max_y);
        emit regionDirty(rect.min_x, rect.min_y, rect.max_x, rect.max_y);
    }
    return ok;
}

bool WorldEditorBridge::paintAt(int x, int y)
{
    if (!m_world || m_activeTool == Hand) return false;
    FfiRect dirty;
    bool ok = matalas_world_paint_at(m_world, x, y, &dirty);
    if (ok) {
        renderRegionToMaster(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
        emit regionDirty(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
        updateUndoState();
    }
    return ok;
}

bool WorldEditorBridge::fillAt(int x, int y)
{
    if (!m_world || m_activeTool == Hand) return false;
    FfiRect dirty;
    bool ok = matalas_world_fill_at(m_world, x, y, &dirty);
    if (ok) {
        renderRegionToMaster(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
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
        emit countryPicked(id, activeCountryName(), activeCountryColor(), activeCountryFlag());
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
        renderRegionToMaster(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
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
        renderRegionToMaster(dirty.min_x, dirty.min_y, dirty.max_x, dirty.max_y);
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

QString WorldEditorBridge::resolveAssetUrl(const QString& relativePath) const
{
    if (relativePath.isEmpty()) return QString();
    if (relativePath.startsWith("qrc:/") || relativePath.startsWith("assets:/") || relativePath.startsWith("file:/")) {
        return relativePath;
    }

    QString stripped = relativePath.startsWith("assets/") ? relativePath.mid(7) : relativePath;

#ifdef Q_OS_ANDROID
    if (QFile::exists("assets:/" + relativePath)) {
        return "assets:/" + relativePath;
    }
    if (QFile::exists("assets:/assets/" + stripped)) {
        return "assets:/assets/" + stripped;
    }
    if (QFile::exists("assets:/" + stripped)) {
        return "assets:/" + stripped;
    }
#endif

    QString appDir = QCoreApplication::applicationDirPath();
    const QStringList localCandidates = {
        QDir(appDir).filePath(relativePath),
        QDir(appDir + "/assets").filePath(stripped),
        QDir(appDir + "/../../..").filePath(relativePath),
        QDir(appDir + "/../..").filePath(relativePath),
        QDir::current().filePath(relativePath),
        "C:/Users/marti/Documents/matalasMap/" + relativePath
    };
    for (const QString& cand : localCandidates) {
        if (QFile::exists(cand)) {
            return QUrl::fromLocalFile(cand).toString();
        }
    }

    return QUrl::fromLocalFile(QDir(appDir).filePath(relativePath)).toString();
}
