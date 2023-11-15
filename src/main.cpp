#include <QApplication>
#include <QSettings>

#include "widgets/MainWindow.h"
#include "core/TraceSOProcess.h"


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName("InstallerCreator");
    QApplication::setApplicationVersion("0.1");
    QApplication::setOrganizationName("PabloPicose");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    // print the settings file path
    qDebug() << "Settings file path: " << QSettings().fileName();

    /*TraceSOProcess traceSOProcess;
    traceSOProcess.setProgramToAnalize(
            "/mnt/masivoFedora/vmShare/ARCOS300/Arcos300m/cmake-build-debug/RMCSClient/RMCSClient");
    traceSOProcess.analize();
    */

    MainWindow w;
    w.show();

    return QApplication::exec();
}
