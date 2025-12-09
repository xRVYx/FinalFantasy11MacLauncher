#include "core/DownloadManifest.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool saveDownloadManifest(const QString& manifestPath, const QMap<QString, DownloadRecord>& records) {
    QJsonArray arr;
    for (auto it = records.constBegin(); it != records.constEnd(); ++it) {
        QJsonObject obj;
        obj.insert("filename", it.value().filename);
        obj.insert("size", static_cast<double>(it.value().size));
        obj.insert("sha256", it.value().sha256);
        obj.insert("verified", it.value().verified);
        arr.append(obj);
    }
    QJsonObject root;
    root.insert("files", arr);

    QJsonDocument doc(root);
    QFile file(manifestPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(doc.toJson(QJsonDocument::Indented)) != -1;
}
