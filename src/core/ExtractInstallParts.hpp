#pragma once

#include <QString>

bool extractInstallParts(const QString& downloadsDir, const QString& installDir, const QString& compatBinary, const QString& prefixPath, QString* errorOut);
