#pragma once

#include <QString>

struct Config {
    QString installPath;
    QString lastProfile;
    QString downloadPath;
    QString compatBinaryPath;
    QString compatPrefixPath;
};

Config loadConfig(const QString& path);
bool saveConfig(const QString& path, const Config& cfg);
