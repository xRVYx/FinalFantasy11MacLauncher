#include "core/LogPath.hpp"

#include <QStandardPaths>

QString logFilePath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/launcher.log";
}
