#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include "bridge/WorldEditorBridge.h"
#include "bridge/MapCanvasItem.h"
#include "bridge/Globe3DItem.h"
#include "bridge/HistoricalNationsModel.h"
#include "bridge/CountryListModel.h"

int main(int argc, char *argv[])
{
    // Enable High-DPI scaling
    QGuiApplication app(argc, argv);
    app.setApplicationName("matalasMap");
    app.setOrganizationName("Matalas");
    app.setOrganizationDomain("matalas.local");

    // Load modern Outfit & Inter typography for crisp UI and maps
    const QStringList fontFiles = {
        "assets/fonts/Outfit-VariableFont_wght.ttf",
        "assets/fonts/Inter-VariableFont.ttf"
    };

    QString loadedFontFamily;
    for (const QString& rel : fontFiles) {
        QStringList candidates = {
            rel,
            "assets:/" + rel,
            "assets:/assets/" + rel.mid(7),
            QDir(QCoreApplication::applicationDirPath()).filePath(rel),
            QDir(QCoreApplication::applicationDirPath() + "/assets").filePath(rel.mid(7)),
            QDir(QCoreApplication::applicationDirPath() + "/../../..").filePath(rel),
            QDir::current().filePath(rel),
            "C:/Users/marti/Documents/matalasMap/" + rel
        };

        for (const QString& cand : candidates) {
            if (QFile::exists(cand)) {
                int id = QFontDatabase::addApplicationFont(cand);
                if (id != -1) {
                    const QStringList families = QFontDatabase::applicationFontFamilies(id);
                    if (!families.isEmpty() && loadedFontFamily.isEmpty()) {
                        loadedFontFamily = families.first();
                        qInfo() << "Loaded modern UI font:" << loadedFontFamily << "from" << cand;
                    }
                    break;
                }
            }
        }
    }

    if (!loadedFontFamily.isEmpty()) {
        QFont defaultFont(loadedFontFamily, 11);
        defaultFont.setStyleStrategy(QFont::PreferAntialias);
        app.setFont(defaultFont);
    } else {
        QFont fallbackFont("Segoe UI", 11);
        fallbackFont.setStyleStrategy(QFont::PreferAntialias);
        app.setFont(fallbackFont);
    }

    qmlRegisterType<MapCanvasItem>("Matalas", 1, 0, "MapCanvas");
    qmlRegisterType<Globe3DItem>("Matalas", 1, 0, "Globe3D");
    qmlRegisterType<WorldEditorBridge>("Matalas", 1, 0, "WorldEditorBridge");
    qmlRegisterType<HistoricalNationsModel>("Matalas", 1, 0, "HistoricalNationsModel");
    qmlRegisterUncreatableType<CountryListModel>("Matalas", 1, 0, "CountryListModel", "Provided by WorldEditorBridge");

    QQmlApplicationEngine engine;

    WorldEditorBridge editorBridge;
    engine.rootContext()->setContextProperty("worldEditor", &editorBridge);

    const QString appDir = QCoreApplication::applicationDirPath();
    engine.rootContext()->setContextProperty("applicationDirPath", appDir);

    // Add QML import paths so plugins and modules are found reliably
    engine.addImportPath(appDir);
    engine.addImportPath(QDir(appDir).filePath("qml"));
    engine.addImportPath(QDir::current().filePath("qml"));

    // Robust search for Main.qml
    QString mainQmlPath;
    const QStringList qmlCandidates = {
        QDir(appDir).filePath("qml/Main.qml"),
        QDir(appDir).filePath("../qml/Main.qml"),
        QDir::current().filePath("qml/Main.qml"),
        QDir::current().filePath("../qml/Main.qml")
    };

    for (const QString& candidate : qmlCandidates) {
        if (QFile::exists(candidate)) {
            mainQmlPath = candidate;
            qInfo() << "Loaded Main.qml from:" << mainQmlPath;
            break;
        }
    }

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [](QObject *obj, const QUrl &objUrl) {
        if (!obj) {
            qCritical() << "Fatal Error: Failed to load QML root object at:" << objUrl;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    if (!mainQmlPath.isEmpty()) {
        engine.load(QUrl::fromLocalFile(mainQmlPath));
    } else {
        engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    }

    return app.exec();
}
