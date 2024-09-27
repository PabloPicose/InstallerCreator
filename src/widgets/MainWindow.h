//
// Created by pablo on 11/3/23.
//

#ifndef INSTALLERCREATOR_MAINWINDOW_H
#define INSTALLERCREATOR_MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QStandardItemModel>
#include "core/BinaryData.h"


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

    void addBinary(const QString &binaryPath, const QString& destDir, QStandardItem *parentItem, bool findDeps);

    QSet<QStandardItem *> getAllChild(QStandardItem *parentItem) const;

    void installRecursive(QTreeWidgetItem* item, const QString& path);

    void changeThemeFlatGray();

    void saveRecursiveBinaryData(BinaryData *binaryData, QStandardItem *parentItem);

    // function that serves to load in the model the data that is in the binaryData recursively
    void loadRecursiveBinaryData(const BinaryData &binaryData, QStandardItem *parentItem);

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

    void onPbAddFromFilesystemCustomDestClicked();

    void onActionSaveAsClicked();

    void onActionLoadClicked();

    void onActionCheckingToolClicked();

private:
    Ui::MainWindow *ui = nullptr;

    QStandardItemModel *m_model = nullptr;
    QFileSystemModel *m_fileSystemModel = nullptr;
};


#endif //INSTALLERCREATOR_MAINWINDOW_H
