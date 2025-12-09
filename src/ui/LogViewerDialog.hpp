#pragma once

#include <QDialog>
#include <QString>

QDialog* createLogViewerDialog(const QString& logPath, QWidget* parent = nullptr);
