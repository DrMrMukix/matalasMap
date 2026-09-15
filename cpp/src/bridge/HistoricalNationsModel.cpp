#include "HistoricalNationsModel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

HistoricalNationsModel::HistoricalNationsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    // Try multiple possible paths to locate content/nations/nations.json
    QStringList candidates = {
        "content/nations/nations.json",
        "../content/nations/nations.json",
        "../../content/nations/nations.json",
        QDir(QCoreApplication::applicationDirPath()).filePath("content/nations/nations.json"),
        QDir(QCoreApplication::applicationDirPath()).filePath("../content/nations/nations.json"),
        "assets:/content/nations/nations.json",
        ":/content/nations/nations.json"
    };

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            loadNations(path);
            break;
        }
    }
}

void HistoricalNationsModel::loadNations(const QString& jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open nations JSON at:" << jsonPath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isArray()) {
        qWarning() << "Error parsing nations JSON:" << error.errorString();
        return;
    }

    beginResetModel();
    m_allNations.clear();

    QJsonArray array = doc.array();
    for (const QJsonValue& val : array) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        NationData nation;
        nation.id = obj["id"].toString();
        nation.name = obj["name"].toString();
        nation.minYear = obj["min_year"].toInt(1650);
        nation.maxYear = obj["max_year"].toInt(2026);
        nation.flagPath = obj["flag"].toString();
        nation.emblemPath = obj["emblem"].toString();
        nation.source = obj["source"].toString("Wikimedia Commons");
        nation.wikidataId = obj["wikidata_id"].toString();
        nation.license = obj["license"].toString();

        if (obj["color"].isArray()) {
            QJsonArray col = obj["color"].toArray();
            int r = col.size() > 0 ? col[0].toInt() : 200;
            int g = col.size() > 1 ? col[1].toInt() : 60;
            int b = col.size() > 2 ? col[2].toInt() : 60;
            int a = col.size() > 3 ? col[3].toInt() : 255;
            nation.color = QColor(r, g, b, a);
        } else {
            nation.color = QColor(180, 50, 50);
        }

        m_allNations.append(nation);
    }

    applyFilter();
    endResetModel();
    emit countChanged();
}

int HistoricalNationsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_filteredNations.size();
}

QVariant HistoricalNationsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredNations.size())
        return QVariant();

    const NationData& n = m_filteredNations[index.row()];

    switch (role) {
    case IdRole:
        return n.id;
    case NameRole:
        return n.name;
    case MinYearRole:
        return n.minYear;
    case MaxYearRole:
        return n.maxYear;
    case PeriodRole:
        return QString("%1 - %2").arg(n.minYear).arg(n.maxYear);
    case FlagPathRole:
        return n.flagPath;
    case EmblemPathRole:
        return n.emblemPath;
    case ColorRole:
        return n.color;
    case SourceRole:
        return n.source;
    case WikidataIdRole:
        return n.wikidataId;
    case LicenseRole:
        return n.license;
    case Qt::DisplayRole:
        return n.name;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> HistoricalNationsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[MinYearRole] = "minYear";
    roles[MaxYearRole] = "maxYear";
    roles[PeriodRole] = "period";
    roles[FlagPathRole] = "flagPath";
    roles[EmblemPathRole] = "emblemPath";
    roles[ColorRole] = "color";
    roles[SourceRole] = "source";
    roles[WikidataIdRole] = "wikidataId";
    roles[LicenseRole] = "license";
    return roles;
}

void HistoricalNationsModel::setFilterText(const QString& filter)
{
    if (m_filterText == filter) return;
    m_filterText = filter;
    beginResetModel();
    applyFilter();
    endResetModel();
    emit filterTextChanged();
    emit countChanged();
}

void HistoricalNationsModel::setSortMode(int mode)
{
    if (m_sortMode == mode) return;
    m_sortMode = mode;
    beginResetModel();
    applyFilter();
    endResetModel();
    emit sortModeChanged();
}

void HistoricalNationsModel::applyFilter()
{
    m_filteredNations.clear();
    QString query = m_filterText.trimmed().toLower();

    bool isYearFilter = false;
    int yearFilter = query.toInt(&isYearFilter);

    for (const NationData& n : m_allNations) {
        if (query.isEmpty()) {
            m_filteredNations.append(n);
            continue;
        }

        if (isYearFilter) {
            if (yearFilter >= n.minYear && yearFilter <= n.maxYear) {
                m_filteredNations.append(n);
                continue;
            }
        }

        if (n.name.toLower().contains(query) ||
            n.id.toLower().contains(query) ||
            QString::number(n.minYear).contains(query) ||
            QString::number(n.maxYear).contains(query)) {
            m_filteredNations.append(n);
        }
    }

    switch (m_sortMode) {
    case 1: // A-Z
        std::sort(m_filteredNations.begin(), m_filteredNations.end(), [](const NationData& a, const NationData& b) {
            return QString::localeAwareCompare(a.name, b.name) < 0;
        });
        break;
    case 2: // Z-A
        std::sort(m_filteredNations.begin(), m_filteredNations.end(), [](const NationData& a, const NationData& b) {
            return QString::localeAwareCompare(a.name, b.name) > 0;
        });
        break;
    case 3: // Chronological (oldest first)
        std::sort(m_filteredNations.begin(), m_filteredNations.end(), [](const NationData& a, const NationData& b) {
            if (a.minYear != b.minYear) return a.minYear < b.minYear;
            return a.name < b.name;
        });
        break;
    case 4: // Modern first (newest first)
        std::sort(m_filteredNations.begin(), m_filteredNations.end(), [](const NationData& a, const NationData& b) {
            if (a.maxYear != b.maxYear) return a.maxYear > b.maxYear;
            return a.minYear > b.minYear;
        });
        break;
    default:
        break;
    }
}

QVariantMap HistoricalNationsModel::getNation(int row) const
{
    QVariantMap map;
    if (row < 0 || row >= m_filteredNations.size())
        return map;

    const NationData& n = m_filteredNations[row];
    map["id"] = n.id;
    map["name"] = n.name;
    map["minYear"] = n.minYear;
    map["maxYear"] = n.maxYear;
    map["period"] = QString("%1 - %2").arg(n.minYear).arg(n.maxYear);
    map["flagPath"] = n.flagPath;
    map["color"] = n.color;
    map["source"] = n.source;
    map["wikidataId"] = n.wikidataId;
    return map;
}
