#pragma once

#include <QString>
#include <QStringList>

bool runCompatProcess(const QString& binary, const QStringList& args, const QString& workingDir, const QString& prefixPath, QString* errorOut);
