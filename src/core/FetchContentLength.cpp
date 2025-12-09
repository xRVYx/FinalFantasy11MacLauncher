#include "core/FetchContentLength.hpp"

#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>

qint64 fetchContentLength(QNetworkAccessManager& manager, const QUrl& url) {
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = manager.head(req);
    if (!reply) {
        return -1;
    }

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    qint64 length = -1;
    if (reply->error() == QNetworkReply::NoError) {
        QVariant v = reply->header(QNetworkRequest::ContentLengthHeader);
        if (v.isValid()) {
            length = v.toLongLong();
        }
    }
    reply->deleteLater();
    return length;
}
