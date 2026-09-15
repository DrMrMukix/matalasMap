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
    QString flagPath;
    bool isSelected;
};

class CountryListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum CountryRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        PixelCountRole,
        FlagPathRole,
        IsSelectedRole
    };

    explicit CountryListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void syncFromWorld(WorldStateHandle world, uint16_t activeCountryId);
    void setSelectedCountry(uint16_t id);
    uint16_t selectedCountryId() const { return m_selectedId; }

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString& text);

    int sortMode() const { return m_sortMode; }
    void setSortMode(int mode);

    int count() const { return m_visibleItems.size(); }

signals:
    void filterTextChanged();
    void sortModeChanged();
    void countChanged();

private:
    void applyFilterAndSort();

    QVector<CountryItem> m_allItems;
    QVector<CountryItem> m_visibleItems;
    uint16_t m_selectedId = 0;
    QString m_filterText;
    int m_sortMode = 0; // 0: ID, 1: A-Z, 2: Z-A, 3: PixelCount (descending)
};
