#include <QApplication>
#include <QStandardPaths>
#include <QDir>

#include "../core/Config.hpp"
#include "../ui/MainWindow.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configDir);
    const QString configPath = configDir + "/config.json";

    Config cfg = loadConfig(configPath);

    QMainWindow* window = createMainWindow(cfg);
    window->show();

    const int rc = app.exec();
    saveConfig(configPath, cfg);
    return rc;
}
