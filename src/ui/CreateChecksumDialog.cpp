#include "ui/ChecksumDialog.hpp"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

QDialog* createChecksumDialog(const QMap<QString, DownloadRecord>& records, QWidget* parent) {
    auto* dlg = new QDialog(parent);
    dlg->setWindowTitle("Checksum Verification");
    auto* layout = new QVBoxLayout(dlg);

    auto* table = new QTableWidget(dlg);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"File", "Size", "SHA256", "Status"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setRowCount(records.size());

    int row = 0;
    for (auto it = records.constBegin(); it != records.constEnd(); ++it, ++row) {
        table->setItem(row, 0, new QTableWidgetItem(it.value().filename));
        table->setItem(row, 1, new QTableWidgetItem(it.value().size > 0 ? QString::number(it.value().size) : "Unknown"));
        table->setItem(row, 2, new QTableWidgetItem(it.value().sha256.isEmpty() ? "N/A" : it.value().sha256));
        table->setItem(row, 3, new QTableWidgetItem(it.value().verified ? "Verified" : "Pending"));
    }

    auto* closeBtn = new QPushButton("Close", dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    layout->addWidget(table);
    layout->addWidget(closeBtn);
    dlg->resize(700, 400);
    return dlg;
}
