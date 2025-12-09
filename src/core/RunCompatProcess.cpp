#include "core/RunCompatProcess.hpp"

#include "core/AppendLog.hpp"

#include <QProcess>
#include <QProcessEnvironment>

bool runCompatProcess(const QString& binary, const QStringList& args, const QString& workingDir, const QString& prefixPath, QString* errorOut) {
    QProcess proc;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!prefixPath.isEmpty()) {
        env.insert("WINEPREFIX", prefixPath);
    }
    proc.setProcessEnvironment(env);
    proc.setProgram(binary);
    proc.setArguments(args);
    if (!workingDir.isEmpty()) {
        proc.setWorkingDirectory(workingDir);
    }

    QObject::connect(&proc, &QProcess::readyReadStandardOutput, [&proc]() {
        const auto data = proc.readAllStandardOutput();
        if (!data.isEmpty()) {
            appendLog(QString::fromUtf8(data));
        }
    });
    QObject::connect(&proc, &QProcess::readyReadStandardError, [&proc]() {
        const auto data = proc.readAllStandardError();
        if (!data.isEmpty()) {
            appendLog(QString::fromUtf8(data));
        }
    });

    proc.start();
    if (!proc.waitForStarted(10000)) {
        if (errorOut) *errorOut = "Failed to start process";
        return false;
    }
    if (!proc.waitForFinished(600000)) { // 10 minutes cap
        proc.kill();
        if (errorOut) *errorOut = "Process timed out";
        appendLog("Process timed out and was killed.");
        return false;
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        if (errorOut) *errorOut = QString("Process failed (code %1)").arg(proc.exitCode());
        return false;
    }
    return true;
}
