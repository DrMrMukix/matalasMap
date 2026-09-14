#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDir>
#include "bridge/WorldEditorBridge.h"
#include "bridge/MapCanvasItem.h"
#include "bridge/CountryListModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("matalasMap");
    app.setOrganizationName("Matalas");
    app.setOrganizationDomain("matalas.local");

    qmlRegisterType<MapCanvasItem>("Matalas", 1, 0, "MapCanvas");
    qmlRegisterType<WorldEditorBridge>("Matalas", 1, 0, "WorldEditorBridge");
    qmlRegisterUncreatableType<CountryListModel>("Matalas", 1, 0, "CountryListModel", "Provided by WorldEditorBridge");

    QQmlApplicationEngine engine;

    WorldEditorBridge editorBridge;
    engine.rootContext()->setContextProperty("worldEditor", &editorBridge);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    // Also fallback to filesystem path if QRC isn't used
    const QString fsPath = QDir::current().filePath("qml/Main.qml");

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    if (QFile::exists("qml/Main.qml")) {
        engine.load(QUrl::fromLocalFile(fsPath));
    } else {
        engine.load(url);
    }

    return app.exec();
}
