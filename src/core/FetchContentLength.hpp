#pragma once

#include <QNetworkAccessManager>
#include <QUrl>

qint64 fetchContentLength(QNetworkAccessManager& manager, const QUrl& url);
