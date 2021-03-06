/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ZypperRPMJob.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QProcess>
#include <QDir>
#include <KShell>
#include <KLocalizedString>

void ZypperRPMJob::executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install)
{
    Q_UNUSED(mimeType)

    if (install) {
        const QString command = QStringLiteral("sudo zypper install %1").arg(KShell::quoteArg(fileInfo.absoluteFilePath()));
        const QString bashCommand = QStringLiteral("echo %1;%1")
            .arg(command, KShell::quoteArg(terminalCloseMessage(install)));
        runScriptInTerminal(QStringLiteral("sh -c %1").arg(KShell::quoteArg(bashCommand)), QDir::homePath());
    } else {
        QProcess rpmInfoProcess;
        rpmInfoProcess.start(QStringLiteral("rpm"), {"-qi", fileInfo.absoluteFilePath()});
        rpmInfoProcess.waitForFinished();
        const QString rpmInfo = rpmInfoProcess.readAll();
        const auto infoMatch = QRegularExpression(QStringLiteral("Name *: (.+)")).match(rpmInfo);
        if (!infoMatch.hasMatch()) {
            Q_EMIT error(i18nc("@info", "Could not resolve package name of %1", fileInfo.baseName()));
        }
        const QString rpmPackageName = KShell::quoteArg(infoMatch.captured(1));
        QProcess *process = new QProcess(this);
        process->start(QStringLiteral("pkexec"), {"zypper", "remove", "--no-confirm", rpmPackageName});
        connectSignals(process);
    }
}
