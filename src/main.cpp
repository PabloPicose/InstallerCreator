#include <QApplication>
#include <QSettings>
#include <QFileInfo>

#include "widgets/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName("InstallerCreator");
    QApplication::setApplicationVersion("0.1");
    QApplication::setOrganizationName("PabloPicose");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    // print the settings file path
    qDebug() << "Settings file path: " << QSettings().fileName();


    MainWindow w;
    w.show();

    return QApplication::exec();
}
