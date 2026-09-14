#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QVector>
#include "matalas_ffi.h"

struct CountryItem {
    uint16_t id;
    QString name;
    QColor color;
    uint64_t pixelCount;
    bool isSelected;
};

class CountryListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum CountryRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        PixelCountRole,
        IsSelectedRole
    };

    explicit CountryListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void syncFromWorld(WorldStateHandle world, uint16_t activeCountryId);
    void setSelectedCountry(uint16_t id);
    uint16_t selectedCountryId() const { return m_selectedId; }

private:
    QVector<CountryItem> m_items;
    uint16_t m_selectedId = 0;
};
