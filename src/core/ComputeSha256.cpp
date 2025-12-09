#include "core/ComputeSha256.hpp"

#include <QCryptographicHash>
#include <QFile>

QString computeSha256(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    while (!file.atEnd()) {
        hasher.addData(file.read(8192));
    }
    return hasher.result().toHex();
}
