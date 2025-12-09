#pragma once

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QString>

QNetworkReply* startDownloadFile(QNetworkAccessManager& manager, const QUrl& url, const QString& destPath, bool resumeExisting);

QNetworkReply* startDownloadFile(QNetworkAccessManager& manager, const QUrl& url, const QString& destPath);
