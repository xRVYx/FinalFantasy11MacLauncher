#include "ui/MainWindow.hpp"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QVector>
#include <functional>
#include <QPointer>
#include <QMap>
#include <memory>

#include "../core/StartDownloadFile.hpp"
#include "../core/ExtractInstallParts.hpp"
#include "../core/LaunchGame.hpp"
#include "../core/FetchContentLength.hpp"
#include "../core/ComputeSha256.hpp"
#include "../core/DownloadManifest.hpp"
#include "../core/AppendLog.hpp"
#include "../core/LogPath.hpp"
#include "ui/LogViewerDialog.hpp"
#include "ui/ChecksumDialog.hpp"

namespace {
struct DownloadItem {
    QString url;
    QString filename;
    qint64 expectedSize;
    QString expectedSha;
};
} // namespace

QMainWindow* createMainWindow(Config& cfg, QWidget* parent) {
    auto* window = new QMainWindow(parent);
    window->setWindowTitle("FFXI Mac Launcher (Preview)");

    auto* central = new QWidget(window);
    auto* layout = new QVBoxLayout(central);

    auto* status = new QLabel(cfg.installPath.isEmpty()
                                  ? "FFXI not installed"
                                  : "Install path: " + cfg.installPath,
                              central);
    auto* downloadDirLabel = new QLabel("Download dir: " + (cfg.downloadPath.isEmpty() ? QString("<default>") : cfg.downloadPath), central);
    auto* compatLabel = new QLabel(cfg.compatBinaryPath.isEmpty() ? "Compat: not set" : "Compat: " + cfg.compatBinaryPath, central);
    auto* prefixLabel = new QLabel(cfg.compatPrefixPath.isEmpty() ? "Prefix: not set" : "Prefix: " + cfg.compatPrefixPath, central);
    auto* downloadProgress = new QProgressBar(central);
    downloadProgress->setRange(0, 100);
    downloadProgress->setValue(0);
    downloadProgress->setTextVisible(true);

    auto* totalProgress = new QProgressBar(central);
    totalProgress->setRange(0, 100);
    totalProgress->setValue(0);
    totalProgress->setTextVisible(true);

    auto* overallLabel = new QLabel("No downloads in progress", central);
    auto* errorLabel = new QLabel("", central);

    auto* installBtn = new QPushButton("Download All Installer Parts", central);
    auto* cancelBtn = new QPushButton("Cancel Download", central);
    auto* retryBtn = new QPushButton("Retry Downloads", central);
    auto* chooseDownloadBtn = new QPushButton("Choose Download Directory", central);
    auto* choosePathBtn = new QPushButton("Choose Install Directory", central);
    auto* chooseCompatBtn = new QPushButton("Choose Wine/GPTK Binary", central);
    auto* choosePrefixBtn = new QPushButton("Choose Prefix Directory", central);
    auto* viewChecksumsBtn = new QPushButton("Checksum Report", central);
    auto* viewLogsBtn = new QPushButton("View Logs", central);
    auto* playBtn = new QPushButton("Play", central);
    playBtn->setEnabled(!cfg.installPath.isEmpty());

    layout->addWidget(status);
    layout->addWidget(downloadDirLabel);
    layout->addWidget(compatLabel);
    layout->addWidget(prefixLabel);
    layout->addWidget(overallLabel);
    layout->addWidget(downloadProgress);
    layout->addWidget(totalProgress);
    layout->addWidget(errorLabel);
    layout->addWidget(installBtn);
    layout->addWidget(cancelBtn);
    layout->addWidget(retryBtn);
    layout->addWidget(chooseDownloadBtn);
    layout->addWidget(choosePathBtn);
    layout->addWidget(chooseCompatBtn);
    layout->addWidget(choosePrefixBtn);
    layout->addWidget(viewChecksumsBtn);
    layout->addWidget(viewLogsBtn);
    layout->addWidget(playBtn);
    layout->addStretch(1);

    const QString defaultDownloadsDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
    if (cfg.downloadPath.isEmpty()) {
        cfg.downloadPath = defaultDownloadsDir;
    }
    auto* downloadsDir = new QString(cfg.downloadPath);
    const QString manifestPath = (*downloadsDir) + "/manifest.json";
    const QString logPath = logFilePath();

    auto manifest = new QMap<QString, DownloadRecord>(loadDownloadManifest(manifestPath));

    auto* manager = new QNetworkAccessManager(window);
    auto* downloads = new QVector<DownloadItem>({
        {"https://gdl.square-enix.com/ffxi/download/us/FFXIFullSetup_US.part1.exe", "FFXIFullSetup_US.part1.exe", -1, ""},
        {"https://gdl.square-enix.com/ffxi/download/us/FFXIFullSetup_US.part2.rar", "FFXIFullSetup_US.part2.rar", -1, ""},
        {"https://gdl.square-enix.com/ffxi/download/us/FFXIFullSetup_US.part3.rar", "FFXIFullSetup_US.part3.rar", -1, ""},
        {"https://gdl.square-enix.com/ffxi/download/us/FFXIFullSetup_US.part4.rar", "FFXIFullSetup_US.part4.rar", -1, ""},
        {"https://gdl.square-enix.com/ffxi/download/us/FFXIFullSetup_US.part5.rar", "FFXIFullSetup_US.part5.rar", -1, ""},
    });

    auto* isDownloading = new bool(false);
    auto* currentIndex = new int(0);
    auto* currentReply = new QPointer<QNetworkReply>();

    auto startNext = std::make_shared<std::function<void()>>();
    *startNext = [=]() {
        if (*currentIndex >= downloads->size()) {
            *isDownloading = false;
            overallLabel->setText("All parts downloaded.");
            downloadProgress->setValue(100);
            totalProgress->setValue(100);
            errorLabel->setText("");
            if (!cfg.installPath.isEmpty()) {
                appendLog("Starting extraction into " + cfg.installPath);
                QString extractError;
                const bool ok = extractInstallParts(*downloadsDir, cfg.installPath, cfg.compatBinaryPath, cfg.compatPrefixPath, &extractError);
                if (ok) {
                    status->setText("Install path: " + cfg.installPath + " (ready)");
                    appendLog("Extraction completed.");
                } else {
                    errorLabel->setText("Extraction failed: " + (extractError.isEmpty() ? QString("see logs") : extractError));
                    appendLog("Extraction failed.");
                }
            }
            saveDownloadManifest(manifestPath, *manifest);
            return;
        }

        DownloadItem item = downloads->at(*currentIndex);
        if (manifest->contains(item.filename)) {
            const auto rec = manifest->value(item.filename);
            if (rec.size > 0) item.expectedSize = rec.size;
            if (!rec.sha256.isEmpty()) item.expectedSha = rec.sha256;
        }

        const QString targetPath = (*downloadsDir) + "/" + item.filename;
        QDir().mkpath(*downloadsDir);

        if (item.expectedSize <= 0) {
            const qint64 len = fetchContentLength(*manager, QUrl(item.url));
            if (len > 0) {
                item.expectedSize = len;
            }
        }

        QFileInfo info(targetPath);
        if (info.exists() && item.expectedSize > 0 && info.size() == item.expectedSize) {
            const QString sha = computeSha256(targetPath);
            if (item.expectedSha.isEmpty()) {
                item.expectedSha = sha;
            }
            if (item.expectedSha == sha) {
                (*manifest)[item.filename] = DownloadRecord{item.filename, info.size(), sha, true};
                saveDownloadManifest(manifestPath, *manifest);
                ++(*currentIndex);
                totalProgress->setValue(static_cast<int>((static_cast<double>(*currentIndex) / downloads->size()) * 100.0));
                (*startNext)();
                return;
            }
        }

        overallLabel->setText(QString("Downloading %1 of %2: %3")
                                  .arg(*currentIndex + 1)
                                  .arg(downloads->size())
                                  .arg(item.filename));
        downloadProgress->setValue(0);
        errorLabel->setText("");

        const bool resumeExisting = info.exists();
        QNetworkReply* reply = startDownloadFile(*manager, QUrl(item.url), targetPath, resumeExisting);
        if (!reply) {
            overallLabel->setText("Unable to start download.");
            errorLabel->setText("Failed to open target for writing.");
            *isDownloading = false;
            return;
        }
        *currentReply = reply;
        appendLog("Downloading " + item.filename + (resumeExisting ? " (resuming)" : ""));

        QObject::connect(reply, &QNetworkReply::downloadProgress, downloadProgress, [downloadProgress, reply](qint64 bytesReceived, qint64 bytesTotal) {
            const qint64 existing = reply->property("existingSize").toLongLong();
            const qint64 total = (bytesTotal > 0) ? bytesTotal + existing : bytesTotal;
            if (total > 0) {
                downloadProgress->setValue(static_cast<int>(((existing + bytesReceived) * 100) / total));
            }
        });

        QObject::connect(reply, &QNetworkReply::downloadProgress, totalProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
            const qint64 existing = reply->property("existingSize").toLongLong();
            const qint64 totalBytes = (bytesTotal > 0) ? bytesTotal + existing : bytesTotal;
            double currentFileFrac = 0.0;
            if (totalBytes > 0) {
                currentFileFrac = static_cast<double>(existing + bytesReceived) / static_cast<double>(totalBytes);
            }
            const double overallFrac = (static_cast<double>(*currentIndex) + currentFileFrac) / static_cast<double>(downloads->size());
            totalProgress->setValue(static_cast<int>(overallFrac * 100.0));
        });

        QObject::connect(reply, &QNetworkReply::finished, window, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                const qint64 existing = reply->property("existingSize").toLongLong();
                const QString sha = computeSha256(targetPath);
                if (!item.expectedSha.isEmpty() && !sha.isEmpty() && sha != item.expectedSha) {
                    errorLabel->setText("Checksum mismatch for " + item.filename);
                    overallLabel->setText("Download failed.");
                    appendLog("Checksum mismatch for " + item.filename);
                    *isDownloading = false;
                } else {
                    const qint64 diskSize = QFileInfo(targetPath).size();
                    (*manifest)[item.filename] = DownloadRecord{item.filename, diskSize, sha, true};
                    saveDownloadManifest(manifestPath, *manifest);
                    appendLog("Download complete: " + item.filename);
                    ++(*currentIndex);
                (*startNext)();
                }
                reply->deleteLater();
            } else {
                overallLabel->setText("Download failed.");
                errorLabel->setText(reply->errorString());
                appendLog("Download failed for " + item.filename + ": " + reply->errorString());
                *isDownloading = false;
                reply->deleteLater();
                *currentReply = nullptr;
            }
        });
    };

    QObject::connect(installBtn, &QPushButton::clicked, window, [=]() {
        if (*isDownloading) {
            overallLabel->setText("Download already in progress...");
            return;
        }
        *isDownloading = true;
        *currentIndex = 0;
        totalProgress->setValue(0);
                (*startNext)();
    });

    QObject::connect(cancelBtn, &QPushButton::clicked, window, [=]() {
        if (*isDownloading && currentReply->data()) {
            currentReply->data()->abort();
            overallLabel->setText("Download canceled.");
            errorLabel->setText("");
            appendLog("Download canceled by user.");
            *isDownloading = false;
        }
    });

    QObject::connect(retryBtn, &QPushButton::clicked, window, [=]() {
        if (*isDownloading) {
            overallLabel->setText("Download in progress; wait or cancel first.");
            return;
        }
        *currentIndex = 0;
        totalProgress->setValue(0);
        downloadProgress->setValue(0);
        errorLabel->setText("");
        *isDownloading = true;
        appendLog("Retrying downloads.");
                (*startNext)();
    });

    QObject::connect(chooseDownloadBtn, &QPushButton::clicked, window, [=, &cfg]() {
        const QString dir = QFileDialog::getExistingDirectory(window, "Choose Download Directory", downloadsDir->isEmpty() ? QDir::homePath() : *downloadsDir);
        if (!dir.isEmpty()) {
            *downloadsDir = dir;
            cfg.downloadPath = dir;
            saveDownloadManifest(*downloadsDir + "/manifest.json", *manifest);
            downloadDirLabel->setText("Download dir: " + *downloadsDir);
            overallLabel->setText("Download directory: " + *downloadsDir);
            appendLog("Download directory set to " + *downloadsDir);
        }
    });

    QObject::connect(choosePathBtn, &QPushButton::clicked, window, [=, &cfg]() {
        const QString dir = QFileDialog::getExistingDirectory(window, "Choose FFXI Install Directory", cfg.installPath.isEmpty() ? QDir::homePath() : cfg.installPath);
        if (!dir.isEmpty()) {
            cfg.installPath = dir;
            status->setText("Install path: " + cfg.installPath);
            playBtn->setEnabled(true);
        }
    });

    QObject::connect(chooseCompatBtn, &QPushButton::clicked, window, [=, &cfg]() {
        const QString file = QFileDialog::getOpenFileName(window, "Choose Wine/GPTK Binary", cfg.compatBinaryPath.isEmpty() ? "/usr/local/bin" : cfg.compatBinaryPath);
        if (!file.isEmpty()) {
            cfg.compatBinaryPath = file;
            compatLabel->setText("Compat: " + cfg.compatBinaryPath);
            appendLog("Compat binary set to " + cfg.compatBinaryPath);
        }
    });

    QObject::connect(choosePrefixBtn, &QPushButton::clicked, window, [=, &cfg]() {
        const QString dir = QFileDialog::getExistingDirectory(window, "Choose Compat Prefix Directory", cfg.compatPrefixPath.isEmpty() ? QDir::homePath() : cfg.compatPrefixPath);
        if (!dir.isEmpty()) {
            cfg.compatPrefixPath = dir;
            prefixLabel->setText("Prefix: " + cfg.compatPrefixPath);
            appendLog("Compat prefix set to " + cfg.compatPrefixPath);
        }
    });

    QObject::connect(viewLogsBtn, &QPushButton::clicked, window, [=]() {
        auto* dlg = createLogViewerDialog(logPath, window);
        dlg->exec();
        dlg->deleteLater();
    });

    QObject::connect(viewChecksumsBtn, &QPushButton::clicked, window, [=]() {
        auto* dlg = createChecksumDialog(*manifest, window);
        dlg->exec();
        dlg->deleteLater();
    });

    QObject::connect(playBtn, &QPushButton::clicked, window, [=, &cfg]() {
        if (!launchGame(cfg)) {
            errorLabel->setText("Launch stub failed: ensure install path exists.");
            appendLog("Launch failed (stub).");
        } else {
            errorLabel->setText("");
            appendLog("Launch triggered (stub).");
        }
    });

    window->setCentralWidget(central);
    window->resize(520, 360);
    return window;
}
