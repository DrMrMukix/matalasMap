#include "CountryListModel.h"
#include <algorithm>

CountryListModel::CountryListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int CountryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_visibleItems.size();
}

QVariant CountryListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_visibleItems.size()) {
        return QVariant();
    }

    const auto& item = m_visibleItems.at(index.row());
    switch (role) {
    case IdRole:
        return item.id;
    case NameRole:
        return item.name;
    case ColorRole:
        return item.color;
    case PixelCountRole:
        return (qulonglong)item.pixelCount;
    case FlagPathRole:
        return item.flagPath;
    case IsSelectedRole:
        return item.isSelected;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CountryListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "countryId";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    roles[PixelCountRole] = "pixelCount";
    roles[FlagPathRole] = "flagPath";
    roles[IsSelectedRole] = "isSelected";
    return roles;
}

void CountryListModel::syncFromWorld(WorldStateHandle world, uint16_t activeCountryId)
{
    m_allItems.clear();
    m_selectedId = activeCountryId;

    if (world) {
        size_t count = matalas_world_get_country_count(world);
        m_allItems.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            FfiCountryInfo info;
            if (matalas_world_get_country_info(world, i, &info)) {
                CountryItem item;
                item.id = info.id;
                item.name = QString::fromUtf8(info.name);
                item.color = QColor(info.r, info.g, info.b, info.a);
                item.pixelCount = info.pixel_count;
                item.flagPath = QString::fromUtf8(info.flag_path);
                item.isSelected = (info.id == activeCountryId);
                m_allItems.append(item);
            }
        }
    }

    applyFilterAndSort();
}

void CountryListModel::setSelectedCountry(uint16_t id)
{
    if (m_selectedId == id) return;
    m_selectedId = id;

    for (int i = 0; i < m_allItems.size(); ++i) {
        m_allItems[i].isSelected = (m_allItems[i].id == id);
    }

    for (int i = 0; i < m_visibleItems.size(); ++i) {
        bool was = m_visibleItems[i].isSelected;
        bool now = (m_visibleItems[i].id == id);
        if (was != now) {
            m_visibleItems[i].isSelected = now;
            emit dataChanged(index(i), index(i), {IsSelectedRole});
        }
    }
}

void CountryListModel::setFilterText(const QString& text)
{
    if (m_filterText == text) return;
    m_filterText = text;
    applyFilterAndSort();
    emit filterTextChanged();
}

void CountryListModel::setSortMode(int mode)
{
    if (m_sortMode == mode) return;
    m_sortMode = mode;
    applyFilterAndSort();
    emit sortModeChanged();
}

void CountryListModel::applyFilterAndSort()
{
    beginResetModel();
    m_visibleItems.clear();

    QString query = m_filterText.trimmed();
    for (const auto& item : m_allItems) {
        if (query.isEmpty() || item.name.contains(query, Qt::CaseInsensitive)) {
            m_visibleItems.append(item);
        }
    }

    // Sort according to m_sortMode
    switch (m_sortMode) {
    case 1: // Alphabetical A-Z
        std::sort(m_visibleItems.begin(), m_visibleItems.end(), [](const CountryItem& a, const CountryItem& b) {
            return QString::localeAwareCompare(a.name, b.name) < 0;
        });
        break;
    case 2: // Alphabetical Z-A
        std::sort(m_visibleItems.begin(), m_visibleItems.end(), [](const CountryItem& a, const CountryItem& b) {
            return QString::localeAwareCompare(a.name, b.name) > 0;
        });
        break;
    case 3: // Territory / Pixel count descending
        std::sort(m_visibleItems.begin(), m_visibleItems.end(), [](const CountryItem& a, const CountryItem& b) {
            if (a.pixelCount != b.pixelCount) return a.pixelCount > b.pixelCount;
            return a.id < b.id;
        });
        break;
    default: // Original ID
        std::sort(m_visibleItems.begin(), m_visibleItems.end(), [](const CountryItem& a, const CountryItem& b) {
            return a.id < b.id;
        });
        break;
    }

    endResetModel();
    emit countChanged();
}
