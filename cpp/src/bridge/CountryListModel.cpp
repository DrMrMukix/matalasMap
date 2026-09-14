#include "CountryListModel.h"

CountryListModel::CountryListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int CountryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant CountryListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return QVariant();
    }

    const auto& item = m_items.at(index.row());
    switch (role) {
    case IdRole:
        return item.id;
    case NameRole:
        return item.name;
    case ColorRole:
        return item.color;
    case PixelCountRole:
        return (qulonglong)item.pixelCount;
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
    roles[IsSelectedRole] = "isSelected";
    return roles;
}

void CountryListModel::syncFromWorld(WorldStateHandle world, uint16_t activeCountryId)
{
    beginResetModel();
    m_items.clear();
    m_selectedId = activeCountryId;

    if (world) {
        size_t count = matalas_world_get_country_count(world);
        m_items.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            FfiCountryInfo info;
            if (matalas_world_get_country_info(world, i, &info)) {
                CountryItem item;
                item.id = info.id;
                item.name = QString::fromUtf8(info.name);
                item.color = QColor(info.r, info.g, info.b, info.a);
                item.pixelCount = info.pixel_count;
                item.isSelected = (info.id == activeCountryId);
                m_items.append(item);
            }
        }
    }

    endResetModel();
}

void CountryListModel::setSelectedCountry(uint16_t id)
{
    if (m_selectedId == id) return;
    m_selectedId = id;

    for (int i = 0; i < m_items.size(); ++i) {
        bool was = m_items[i].isSelected;
        bool now = (m_items[i].id == id);
        if (was != now) {
            m_items[i].isSelected = now;
            emit dataChanged(index(i), index(i), {IsSelectedRole});
        }
    }
}
