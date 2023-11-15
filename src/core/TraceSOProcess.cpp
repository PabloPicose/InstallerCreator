//
// Created by pablo on 11/15/23.
//

#include "TraceSOProcess.h"

#include <QDebug>
#include <QRegularExpression>

TraceSOProcess::TraceSOProcess(QObject *parent) : QProcess(parent) {
    createConnectionsDefault();
    setProcessChannelMode(QProcess::MergedChannels);
}

TraceSOProcess::~TraceSOProcess() {

}

void TraceSOProcess::setProgramToAnalize(const QString &program) {
    m_programToAnalize = program;
}

void TraceSOProcess::analize() {

    // strace -e openat $COMMAND 2>&1 | awk '/^openat\(.*\.so/ && !/= -1/ {print $2}'
    setProgram("strace");
    setArguments(
            {"-e", "openat", m_programToAnalize});
    start();
}

void TraceSOProcess::createConnectionsDefault() {
    connect(this, &QProcess::readyReadStandardOutput, this, &TraceSOProcess::onReadyReadStandardOutput);
    connect(this, &QProcess::readyReadStandardError, this, &TraceSOProcess::onReadyReadStandardError);
    connect(this, &QProcess::finished, this, &TraceSOProcess::onFinished);
    connect(this, &QProcess::errorOccurred, this, &TraceSOProcess::onErrorOccurred);
    connect(this, &QProcess::stateChanged, this, &TraceSOProcess::onStateChanged);
}

void TraceSOProcess::onReadyReadStandardOutput() {
    const auto byteArr = readAll();
    for (char c : byteArr)
    {
        if (c == '\n')
        {
            //qDebug() << "STDOUT Line: " << m_bufferStdOut;

            // We are going to create a RegExp to match the line.
            // Example of output: Match:  "openat(AT_FDCWD, \"/mnt/masivoFedora/Qt/6.2.4/gcc_64/plugins/imageformats/libqgif.so.avx2\", O_RDONLY|O_CLOEXEC) = -1 ENOENT (No such file or directory)"
            //
            // The RegExp will pass only if the line contains the string ".so" and does not contain the string "= -1"
            const QRegularExpression re("(?=.*openat)(?=.*\\.so)(?!.*= -1)");
            const QRegularExpressionMatch match = re.match(m_bufferStdOut);
            if (match.hasMatch())
            {
                //qDebug().noquote() << "Match: " << m_bufferStdOut;
                // Now we are going to extract the path of the .so file
                // example input: openat(AT_FDCWD, "/mnt/masivoFedora/Qt/6.2.4/gcc_64/lib/libQt6WebChannel.so.6", O_RDONLY|O_CLOEXEC) = 3
                const QRegularExpression regPath("\"(.*?)\"");
                const QRegularExpressionMatch matchPath = regPath.match(m_bufferStdOut);
                if (matchPath.hasMatch())
                {
                    const QString filePath = matchPath.captured(1);
                    qDebug().noquote() << "MatchPath: " << filePath;
                }
                else
                {
                    qDebug() << "No matchPath: " << m_bufferStdOut;
                }

            }
            else {
                //qDebug() << "No match: " << m_bufferStdOut;
            }

            m_bufferStdOut.clear();
        }
        else
        {
            m_bufferStdOut.append(c);
        }
    }
}

void TraceSOProcess::onReadyReadStandardError() {
    const auto byteArr = readAllStandardError();
    for (char c : byteArr)
    {
        if (c == '\n')
        {
            qDebug() << "STDERR Line: " << m_bufferStdErr;
            m_bufferStdErr.clear();
        }
        else
        {
            m_bufferStdErr.append(c);
        }
    }
}

void TraceSOProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "TraceSOProcess::onFinished" << exitCode << exitStatus;
}

void TraceSOProcess::onErrorOccurred(QProcess::ProcessError error) {
    qDebug() << "TraceSOProcess::onErrorOccurred" << errorString();
}

void TraceSOProcess::onStateChanged(QProcess::ProcessState newState) {
    qDebug() << "TraceSOProcess::onStateChanged" << newState;
}

void TraceSOProcess::appendSoFile(const QString &soFile) {
    QWriteLocker locker(&m_lock);
    m_soFiles.append(soFile);
    locker.unlock();
}
