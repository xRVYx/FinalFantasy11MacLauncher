#include "core/DownloadManifest.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QMap<QString, DownloadRecord> loadDownloadManifest(const QString& manifestPath) {
    QMap<QString, DownloadRecord> records;
    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return records;
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return records;
    }
    const auto arr = doc.object().value("files").toArray();
    for (const auto& value : arr) {
        const auto obj = value.toObject();
        DownloadRecord rec;
        rec.filename = obj.value("filename").toString();
        rec.size = obj.value("size").toVariant().toLongLong();
        rec.sha256 = obj.value("sha256").toString();
        rec.verified = obj.value("verified").toBool();
        if (!rec.filename.isEmpty()) {
            records.insert(rec.filename, rec);
        }
    }
    return records;
}
