#pragma once

#include <QMap>
#include <QString>

struct DownloadRecord {
    QString filename;
    qint64 size = -1;
    QString sha256;
    bool verified = false;
};

QMap<QString, DownloadRecord> loadDownloadManifest(const QString& manifestPath);
bool saveDownloadManifest(const QString& manifestPath, const QMap<QString, DownloadRecord>& records);
