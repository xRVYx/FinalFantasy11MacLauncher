#include "core/LaunchGame.hpp"

#include "core/AppendLog.hpp"
#include "core/RunCompatProcess.hpp"

#include <QProcess>
#include <QFileInfo>

bool launchGame(const Config& cfg) {
    if (cfg.installPath.isEmpty() || !QFileInfo::exists(cfg.installPath)) {
        appendLog("Launch failed: install path missing.");
        return false;
    }
    if (cfg.compatBinaryPath.isEmpty() || !QFileInfo::exists(cfg.compatBinaryPath)) {
        appendLog("Launch failed: compat binary missing.");
        return false;
    }

    QString error;
    // Placeholder launch command; real game exe path should be configured (POL/FFXI main executable).
    const QString gameExe = cfg.installPath + "/pol.exe";
    QStringList args;
    args << gameExe;

    const bool ok = runCompatProcess(cfg.compatBinaryPath, args, cfg.installPath, cfg.compatPrefixPath, &error);
    if (!ok) {
        appendLog("Launch failed: " + error);
    }
    return ok;
}
