//
// Created by pablo on 11/15/23.
//

#ifndef INSTALLERCREATOR_TRACESOPROCESS_H
#define INSTALLERCREATOR_TRACESOPROCESS_H

#include <QProcess>
#include <QReadWriteLock>
#include <QFutureWatcher>

class TraceSOProcess : public QProcess {
Q_OBJECT
public:
    explicit TraceSOProcess(QObject *parent = nullptr);

    ~TraceSOProcess() override;

    void setProgramToAnalize(const QString& program);

    void analize();

private slots:
    void onReadyReadStandardOutput();

    void onReadyReadStandardError();

    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onErrorOccurred(QProcess::ProcessError error);

    void onStateChanged(QProcess::ProcessState newState);

private:
    void createConnectionsDefault();

    void appendSoFile(const QString& soFile);

private:
    QString m_programToAnalize;

    QByteArray m_bufferStdOut;
    QByteArray m_bufferStdErr;

    QByteArray m_buffer;

    QStringList m_soFiles;

    mutable QReadWriteLock m_lock;
};


#endif //INSTALLERCREATOR_TRACESOPROCESS_H
