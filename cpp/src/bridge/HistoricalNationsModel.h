#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QVector>
#include <QJsonObject>

struct NationData {
    QString id;
    QString name;
    int minYear;
    int maxYear;
    QString flagPath;
    QString emblemPath;
    QColor color;
    QString source;
    QString wikidataId;
    QString license;
};

class HistoricalNationsModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum NationRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        MinYearRole,
        MaxYearRole,
        PeriodRole,
        FlagPathRole,
        EmblemPathRole,
        ColorRole,
        SourceRole,
        WikidataIdRole,
        LicenseRole
    };

    explicit HistoricalNationsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString& filter);

    int sortMode() const { return m_sortMode; }
    void setSortMode(int mode);

    void loadNations(const QString& jsonPath);

public slots:
    QVariantMap getNation(int row) const;

signals:
    void filterTextChanged();
    void sortModeChanged();
    void countChanged();

private:
    void applyFilter();

    QVector<NationData> m_allNations;
    QVector<NationData> m_filteredNations;
    QString m_filterText;
    int m_sortMode = 0;
};
