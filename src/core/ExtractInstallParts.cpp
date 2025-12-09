#include "core/ExtractInstallParts.hpp"

#include "core/AppendLog.hpp"
#include "core/RunCompatProcess.hpp"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

namespace {
bool runProcess(const QString& program, const QStringList& args, int timeoutMs, QString* errorOut) {
    QProcess proc;
    proc.setProgram(program);
    proc.setArguments(args);
    proc.start();
    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        if (errorOut) *errorOut = "Process timed out";
        return false;
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        if (errorOut) *errorOut = QString("Process failed: %1").arg(proc.exitCode());
        return false;
    }
    return true;
}
} // namespace

bool extractInstallParts(const QString& downloadsDir, const QString& installDir, const QString& compatBinary, const QString& prefixPath, QString* errorOut) {
    QDir().mkpath(installDir);

    const QString sevenZip = QStandardPaths::findExecutable("7za");
    const QString unar = QStandardPaths::findExecutable("unar");

    const QString part1 = downloadsDir + "/FFXIFullSetup_US.part1.exe";
    const QString part2 = downloadsDir + "/FFXIFullSetup_US.part2.rar";

    // Preferred: 7zip to extract the SFX archive (part1 handles volumes).
    if (!sevenZip.isEmpty() && QFileInfo::exists(part1)) {
        appendLog("Running 7za extraction on " + part1);
        QString err;
        if (runProcess(sevenZip, QStringList() << "x" << "-o" + installDir << part1, 900000, &err)) {
            return QFileInfo::exists(installDir) && QFileInfo(installDir).isDir();
        } else {
            appendLog("7za extraction failed: " + err);
            if (errorOut) *errorOut = "7za extraction failed";
        }
    }

    // Fallback: unar on part1 or part2 (multi-part).
    if (!unar.isEmpty()) {
        if (QFileInfo::exists(part1)) {
            appendLog("Running unar on part1");
            QString err;
            if (runProcess(unar, QStringList() << "-o" << installDir << part1, 900000, &err)) {
                return QFileInfo::exists(installDir) && QFileInfo(installDir).isDir();
            } else {
                appendLog("unar part1 failed: " + err);
                if (errorOut) *errorOut = "unar part1 failed";
            }
        }
        if (QFileInfo::exists(part2)) {
            appendLog("Running unar on part2");
            QString err;
            if (runProcess(unar, QStringList() << "-o" << installDir << part2, 900000, &err)) {
                return QFileInfo::exists(installDir) && QFileInfo(installDir).isDir();
            } else {
                appendLog("unar part2 failed: " + err);
                if (errorOut) *errorOut = "unar part2 failed";
            }
        }
    }

    // Last resort: run the installer via provided compat binary (Wine/GPTK) if available.
    if (!compatBinary.isEmpty() && QFileInfo::exists(part1)) {
        appendLog("Attempting to run installer via compat binary: " + compatBinary);
        QString err;
        if (runCompatProcess(compatBinary, QStringList() << part1, downloadsDir, prefixPath, &err)) {
            return QFileInfo::exists(installDir) && QFileInfo(installDir).isDir();
        } else {
            appendLog("Compat installer run failed: " + err);
            if (errorOut) *errorOut = err;
        }
    }

    return false;
}
