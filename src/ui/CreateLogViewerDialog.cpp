#include "ui/LogViewerDialog.hpp"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFile>

QDialog* createLogViewerDialog(const QString& logPath, QWidget* parent) {
    auto* dlg = new QDialog(parent);
    dlg->setWindowTitle("Launcher Logs");
    auto* layout = new QVBoxLayout(dlg);

    auto* text = new QTextEdit(dlg);
    text->setReadOnly(true);
    QFile file(logPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        text->setPlainText(QString::fromUtf8(file.readAll()));
    } else {
        text->setPlainText("No logs yet.");
    }

    auto* closeBtn = new QPushButton("Close", dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    layout->addWidget(text);
    layout->addWidget(closeBtn);
    dlg->resize(600, 400);
    return dlg;
}
