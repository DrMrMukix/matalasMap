#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QDir>
#include <QVariantList>
#include <QImage>
#include "matalas_ffi.h"
#include "CountryListModel.h"

class WorldEditorBridge : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString worldName READ worldName NOTIFY worldNameChanged)
    Q_PROPERTY(int activeTool READ activeTool WRITE setActiveTool NOTIFY activeToolChanged)
    Q_PROPERTY(int activeMode READ activeMode WRITE setActiveMode NOTIFY activeModeChanged)
    Q_PROPERTY(int displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
    Q_PROPERTY(int brushRadius READ brushRadius WRITE setBrushRadius NOTIFY brushRadiusChanged)
    Q_PROPERTY(int activeCountryId READ activeCountryId WRITE setActiveCountryId NOTIFY activeCountryChanged)
    Q_PROPERTY(QString activeCountryName READ activeCountryName NOTIFY activeCountryChanged)
    Q_PROPERTY(QColor activeCountryColor READ activeCountryColor NOTIFY activeCountryChanged)
    Q_PROPERTY(QString activeCountryFlag READ activeCountryFlag NOTIFY activeCountryChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(CountryListModel* countries READ countries CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString loadingMessage READ loadingMessage NOTIFY loadingMessageChanged)

public:
    enum Tool {
        Brush = 0,
        Eraser = 1,
        Fill = 2,
        Picker = 3,
        Hand = 4
    };
    Q_ENUM(Tool)

    enum EditorMode {
        Terrain = 0,
        Political = 1
    };
    Q_ENUM(EditorMode)

    enum DisplayMode {
        FlatColor = 0,
        FlagPattern = 1
    };
    Q_ENUM(DisplayMode)

    explicit WorldEditorBridge(QObject* parent = nullptr);
    ~WorldEditorBridge();

    WorldStateHandle handle() const { return m_world; }

    QString worldName() const { return m_worldName; }
    int activeTool() const { return m_activeTool; }
    int activeMode() const { return m_activeMode; }
    int displayMode() const { return m_displayMode; }
    int brushRadius() const { return m_brushRadius; }
    int activeCountryId() const { return m_activeCountryId; }
    QString activeCountryName() const;
    QColor activeCountryColor() const;
    QString activeCountryFlag() const;
    bool canUndo() const;
    bool canRedo() const;
    CountryListModel* countries() { return m_countryModel; }
    const QImage& worldImage() const { return m_masterWorldImage; }
    QImage& masterWorldImage() { return m_masterWorldImage; }
    void renderRegionToMaster(int minX, int minY, int maxX, int maxY);
    QVector<FfiFlagAnchor> getFlagAnchors() const;
    QString getCountryFlagPath(int id) const;
    Q_INVOKABLE QString resolveAssetUrl(const QString& relativePath) const;

    bool isLoading() const { return m_isLoading; }
    QString loadingMessage() const { return m_loadingMessage; }
    void setIsLoading(bool loading, const QString& msg = QString());

public slots:
    void newWorld(const QString& name, int presetType); // 0 = empty, 1 = earth blank, 2 = earth 2026
    void newWorldWithPreset(const QString& name, int presetType) { newWorld(name, presetType); }
    QString getDefaultSavePath() const;
    bool loadWorld(const QString& filePath);
    bool saveWorld(const QString& filePath);
    bool saveWorldAs(const QString& filePath, const QString& newName);
    bool deleteSavedWorld(const QString& filePath);
    QVariantList getSavedWorldsList();

    // Enhanced save with names and overwrite control
    bool checkSaveExists(const QString& name) const;
    QString getSavePathForName(const QString& name) const;
    bool saveWorldNamed(const QString& name, bool overwrite);

    // Asynchronous loading operations with loading overlay feedback
    void startAsyncNewWorld(const QString& name, int presetType);
    void startAsyncLoadWorld(const QString& filePath);

    void setActiveTool(int tool);
    void setActiveMode(int mode);
    void setDisplayMode(int mode);
    void setBrushRadius(int radius);
    void setActiveCountryId(int id);
    void setCountryFlag(int id, const QString& flagPath);

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
    void displayModeChanged();
    void brushRadiusChanged();
    void activeCountryChanged();
    void canUndoChanged();
    void canRedoChanged();
    void worldLoaded();
    void regionDirty(int minX, int minY, int maxX, int maxY);
    void countryPicked(int id, const QString& name, const QColor& color, const QString& flagPath);
    void isLoadingChanged();
    void loadingMessageChanged();

private:
    void updateUndoState();
    void rasterizeAndSendFlagToRust(int id, const QString& flagPath);

    WorldStateHandle m_world = nullptr;
    CountryListModel* m_countryModel = nullptr;

    QString m_worldName = "Tierra";
    int m_activeTool = 0;      // 0: Brush, 1: Eraser, 2: Fill, 3: Picker, 4: Hand
    int m_activeMode = 0;      // 0: Terrain, 1: Political
    int m_displayMode = 0;     // 0: FlatColor, 1: FlagPattern
    int m_brushRadius = 16;
    int m_activeCountryId = 0;
    bool m_isLoading = false;
    QString m_loadingMessage = "";
    QImage m_masterWorldImage;
};
