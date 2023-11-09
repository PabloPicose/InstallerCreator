//
// Created by pablo on 11/3/23.
//

#ifndef INSTALLERCREATOR_MAINWINDOW_H
#define INSTALLERCREATOR_MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QStandardItemModel>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTreeWidgetItem;
class QFileSystemModel;

class MainWindow : public QMainWindow {
Q_OBJECT

    enum ModelColumn {
        ModelColumnName,
        ModelColumnPath,
        ModelColumnFind,
        ModelColumnDestination
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    static QList<QFileInfo> getDependencies(const QString &binaryPath);

private:
    void createConnectionsDefault();

    void addBinary(const QString &binaryPath, bool onBinDir, QStandardItem *parentItem);

    QSet<QStandardItem *> getAllChild(QStandardItem *parentItem) const;

    void installRecursive(QTreeWidgetItem* item, const QString& path);

private slots:

    void onTbAddBinaryClicked();

    void onPreGenerateButtonClicked();

    void onItemChanged(QStandardItem *item);

    void onTbOutputPathFindClicked();

    void onTbFindBinaryClicked();

    void onPbInstallClicked();

    void onCbZippedToggled(bool checked);

    void onCustomContextMenuRequestedPaths(const QPoint &pos);

    void onPbAddFromFileSystemClicked();

private:
    Ui::MainWindow *ui = nullptr;

    QStandardItemModel *m_model = nullptr;
    QFileSystemModel *m_fileSystemModel = nullptr;
};


#endif //INSTALLERCREATOR_MAINWINDOW_H
