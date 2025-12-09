#pragma once

#include <QDialog>
#include <QMap>
#include "../core/DownloadManifest.hpp"

QDialog* createChecksumDialog(const QMap<QString, DownloadRecord>& records, QWidget* parent = nullptr);
