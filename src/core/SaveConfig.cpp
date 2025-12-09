#include "core/Config.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

bool saveConfig(const QString& path, const Config& cfg) {
    QJsonObject obj;
    obj.insert("installPath", cfg.installPath);
    obj.insert("lastProfile", cfg.lastProfile);
    obj.insert("downloadPath", cfg.downloadPath);
    obj.insert("compatBinaryPath", cfg.compatBinaryPath);
    obj.insert("compatPrefixPath", cfg.compatPrefixPath);

    QJsonDocument doc(obj);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(doc.toJson(QJsonDocument::Indented)) != -1;
}
