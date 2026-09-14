#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QDir>
#include "matalas_ffi.h"
#include "CountryListModel.h"

class WorldEditorBridge : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString worldName READ worldName NOTIFY worldNameChanged)
    Q_PROPERTY(int activeTool READ activeTool WRITE setActiveTool NOTIFY activeToolChanged)
    Q_PROPERTY(int activeMode READ activeMode WRITE setActiveMode NOTIFY activeModeChanged)
    Q_PROPERTY(int brushRadius READ brushRadius WRITE setBrushRadius NOTIFY brushRadiusChanged)
    Q_PROPERTY(int activeCountryId READ activeCountryId WRITE setActiveCountryId NOTIFY activeCountryChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(CountryListModel* countries READ countries CONSTANT)

public:
    explicit WorldEditorBridge(QObject* parent = nullptr);
    ~WorldEditorBridge();

    WorldStateHandle handle() const { return m_world; }

    QString worldName() const { return m_worldName; }
    int activeTool() const { return m_activeTool; }
    int activeMode() const { return m_activeMode; }
    int brushRadius() const { return m_brushRadius; }
    int activeCountryId() const { return m_activeCountryId; }
    bool canUndo() const;
    bool canRedo() const;
    CountryListModel* countries() { return m_countryModel; }

public slots:
    void newWorld(const QString& name, bool earthPreset);
    bool loadWorld(const QString& filePath);
    bool saveWorld(const QString& filePath);

    void setActiveTool(int tool);
    void setActiveMode(int mode);
    void setBrushRadius(int radius);
    void setActiveCountryId(int id);

    int createCountry(const QString& name, const QColor& color);
    bool updateCountry(int id, const QString& name, const QColor& color);
    bool deleteCountry(int id);

    bool paintAt(int x, int y);
    bool fillAt(int x, int y);
    int pickAt(int x, int y);

    bool undo();
    bool redo();

signals:
    void worldNameChanged();
    void activeToolChanged();
    void activeModeChanged();
    void brushRadiusChanged();
    void activeCountryChanged();
    void canUndoChanged();
    void canRedoChanged();
    void worldLoaded();
    void regionDirty(int minX, int minY, int maxX, int maxY);

private:
    void updateUndoState();

    WorldStateHandle m_world = nullptr;
    CountryListModel* m_countryModel = nullptr;

    QString m_worldName = "Tierra";
    int m_activeTool = 0;   // 0: Brush, 1: Eraser, 2: Fill, 3: Picker
    int m_activeMode = 0;   // 0: Terrain, 1: Political
    int m_brushRadius = 16;
    int m_activeCountryId = 0;
};
