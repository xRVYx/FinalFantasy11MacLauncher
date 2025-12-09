#include "core/StartDownloadFile.hpp"

#include <QSaveFile>
#include <QFile>
#include <QObject>
#include <QNetworkRequest>
#include <QFileInfo>
#include <QByteArray>

QNetworkReply* startDownloadFile(QNetworkAccessManager& manager, const QUrl& url, const QString& destPath, bool resumeExisting) {
    QNetworkRequest request(url);

    QIODevice* outDevice = nullptr;
    qint64 existingSize = 0;

    if (resumeExisting && QFileInfo::exists(destPath)) {
        auto* file = new QFile(destPath);
        if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
            delete file;
            return nullptr;
        }
        existingSize = file->size();
        request.setRawHeader("Range", QByteArray("bytes=" + QByteArray::number(existingSize) + "-"));
        outDevice = file;
    } else {
        auto* file = new QSaveFile(destPath);
        if (!file->open(QIODevice::WriteOnly)) {
            file->deleteLater();
            return nullptr;
        }
        outDevice = file;
    }

    auto* reply = manager.get(request);
    if (!reply) {
        outDevice->deleteLater();
        return nullptr;
    }
    reply->setProperty("existingSize", existingSize);

    QObject::connect(reply, &QNetworkReply::readyRead, reply, [reply, outDevice]() {
        outDevice->write(reply->readAll());
    });

    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, outDevice]() {
        if (reply->error() == QNetworkReply::NoError) {
            if (auto* saveFile = qobject_cast<QSaveFile*>(outDevice)) {
                saveFile->commit();
            } else {
                outDevice->close();
            }
        } else {
            if (auto* saveFile = qobject_cast<QSaveFile*>(outDevice)) {
                saveFile->cancelWriting();
            } else {
                outDevice->close();
            }
        }
        outDevice->deleteLater();
    });

    QObject::connect(reply, &QNetworkReply::errorOccurred, reply, [outDevice](QNetworkReply::NetworkError) {
        if (auto* saveFile = qobject_cast<QSaveFile*>(outDevice)) {
            saveFile->cancelWriting();
        } else {
            outDevice->close();
        }
    });

    return reply;
}
