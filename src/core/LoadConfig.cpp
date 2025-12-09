#include "core/Config.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Config loadConfig(const QString& path) {
    Config cfg;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return cfg; // return defaults when no file exists
    }

    const auto data = file.readAll();
    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return cfg;
    }

    const auto obj = doc.object();
    cfg.installPath = obj.value("installPath").toString();
    cfg.lastProfile = obj.value("lastProfile").toString();
    cfg.downloadPath = obj.value("downloadPath").toString();
    cfg.compatBinaryPath = obj.value("compatBinaryPath").toString();
    cfg.compatPrefixPath = obj.value("compatPrefixPath").toString();
    return cfg;
}
