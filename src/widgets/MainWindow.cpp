//
// Created by pablo on 11/3/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "RenamePathDialog.h"
#include "core/BinaryData.h"

#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>
#include <QMenu>
#include <QPersistentModelIndex>


MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    changeThemeFlatGray();

    ui->tv_paths->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create the model
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setRootPath(QDir::rootPath());
    auto filters = m_fileSystemModel->filter();
    filters |= QDir::Hidden | QDir::NoDotAndDotDot | QDir::CaseSensitive;

    m_fileSystemModel->setFilter(filters);
    // Enable the sort on the model

    // Set the model to the tree view
    ui->tv_fileExplorer->setModel(m_fileSystemModel);
    // hide the "Date modified" column
    ui->tv_fileExplorer->hideColumn(3);
    // hide the "size" column
    ui->tv_fileExplorer->hideColumn(1);
    // sort the items by name


    m_model = new QStandardItemModel(this);
    // the model will have three columns, Name, Path, Find, Destination
    m_model->setColumnCount(4);
    m_model->setHeaderData(ModelColumnName, Qt::Horizontal, "Name");
    m_model->setHeaderData(ModelColumnPath, Qt::Horizontal, "Path");
    m_model->setHeaderData(ModelColumnFind, Qt::Horizontal, "Install deps");
    m_model->setHeaderData(ModelColumnDestination, Qt::Horizontal, "Destination");
    ui->tv_paths->setModel(m_model);

    createConnectionsDefault();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onPreGenerateButtonClicked() {
    QSet<QString> binaryPaths; // is the binary path
    QSet<QString> destinations; // is the form that the tree will be organized

    // clean the tree widget
    for (int i = ui->tw_organization->topLevelItemCount(); i >= 0; --i) {
        auto* item = ui->tw_organization->takeTopLevelItem(i);
        delete item;
    }
    // columns: 1. Name, 2. Path

    const auto childs = getAllChild(m_model->invisibleRootItem());
    for (const auto& item : childs) {
        const QModelIndex destinationIndex = item->index().siblingAtColumn(ModelColumnDestination);
        const QModelIndex pathIndex = item->index().siblingAtColumn(ModelColumnPath);
        const QModelIndex nameIndex = item->index().siblingAtColumn(ModelColumnName);

        const auto destination = destinationIndex.data().toString();
        const auto path = pathIndex.data().toString();
        const auto name = nameIndex.data().toString();

        if (binaryPaths.contains(path)) {
            continue;
        }
        binaryPaths.insert(path);
        destinations.insert(destination);
    }

    // for each path now we will have a root item, with childs that continue the path.
    // For example, if a path contains this: bin/platforms/plugins and this other: bin/common and this other: lib
    // In the tree widget we will have:
    // bin
    //   platforms
    //     plugins
    //   common
    // lib
    // So we cannot duplicate the paths, and we will create the tree widget.
    // For the example case, we will only have two root items: bin and lib. bin will have two childs: platforms and common
    // and platforms will have a child: plugins

    for (const auto& dest : destinations) {
        // split into parts
        QStringList parts = dest.split('/');
        QTreeWidgetItem* parent = ui->tw_organization->invisibleRootItem();
        for (const QString part : parts) {
            // if the parent is null, we will create a root item
            if (!parent) {
                parent = new QTreeWidgetItem(ui->tw_organization);
                parent->setText(0, part);
                continue;
            }
            // if the parent is not null, we will search if the part is already a child of the parent
            bool found = false;
            for (int i = 0; i < parent->childCount(); ++i) {
                auto* child = parent->child(i);
                if (child->text(0) == part) {
                    parent = child;
                    found = true;
                    break;
                }
            }
            // if the part is not a child of the parent, we will create a new child
            if (!found) {
                auto* child = new QTreeWidgetItem(parent);
                child->setText(0, part);
                parent = child;
            }
        }
    }
    // now we will loop over the childs, we will take the path and the name, and we will add it to the tree widget.
    for (const auto child : childs) {
        const QModelIndex destinationIndex = child->index().siblingAtColumn(ModelColumnDestination);
        const QModelIndex pathIndex = child->index().siblingAtColumn(ModelColumnPath);
        const QModelIndex nameIndex = child->index().siblingAtColumn(ModelColumnName);

        const auto destination = destinationIndex.data().toString();
        const auto path = pathIndex.data().toString();
        const auto name = nameIndex.data().toString();

        // we will search the destination in the tree widget, if the destination is bin/platforms/plugins
        // we have to take the root item bin, then the child platforms, then the child plugins and add the new item as a child
        // of plugins. All the paths has been previously created so we can consider that the destination exists.
        QStringList parts = destination.split('/');
        QTreeWidgetItem* parent = ui->tw_organization->invisibleRootItem();

        for (const auto& pathPar : parts) {
            for (int i = 0; i < parent->childCount(); ++i) {
                auto* childSearch = parent->child(i);
                if (childSearch->text(0) == pathPar) {
                    parent = childSearch;
                    break;
                }
            }
        }
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, name);
        item->setText(1, path);
    }

    // uncollapse all the items
    ui->tw_organization->expandAll();
    // the first column should be resized to fit the content
    ui->tw_organization->resizeColumnToContents(0);
}

void MainWindow::createConnectionsDefault() {
    connect(ui->pb_preGenerate, &QPushButton::clicked,
            this, &MainWindow::onPreGenerateButtonClicked);

    connect(ui->tb_addBinary, &QPushButton::clicked,
            this, &MainWindow::onTbAddBinaryClicked);

    connect(m_model, &QStandardItemModel::itemChanged,
            this, &MainWindow::onItemChanged);

    connect(ui->tb_outputPathFind, &QPushButton::clicked,
            this, &MainWindow::onTbOutputPathFindClicked);

    connect(ui->tb_findBinary, &QPushButton::clicked,
            this, &MainWindow::onTbFindBinaryClicked);

    connect(ui->pb_install, &QPushButton::clicked,
            this, &MainWindow::onPbInstallClicked);

    connect(ui->tv_paths, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onCustomContextMenuRequestedPaths);

    connect(ui->cb_zipped, &QCheckBox::toggled,
            this, &MainWindow::onCbZippedToggled);

    connect(ui->pb_addFromFileSystem, &QPushButton::clicked,
            this, &MainWindow::onPbAddFromFileSystemClicked);

    connect(ui->pb_addFromFileSystemCustomDest, &QPushButton::clicked,
            this, &MainWindow::onPbAddFromFilesystemCustomDestClicked);

    connect(ui->actionSave_as, &QAction::triggered,
            this, &MainWindow::onActionSaveAsClicked);

    connect(ui->actionLoad, &QAction::triggered,
            this, &MainWindow::onActionLoadClicked);
}

QList<QFileInfo> MainWindow::getDependencies(const QString& binaryPath) {

    QStringList dependencies;

    // Use QProcess to run ldd command
    QProcess process;
    process.setProgram("ldd");
    process.setArguments({ binaryPath });

    // Start the process
    process.start();
    process.waitForFinished();

    // Read the output
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    // Extract the library paths
    for (const QString& line : lines) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.length() > 2) {
            dependencies.append(parts[2]);
        }
    }


    QList<QFileInfo> dependenciesFileInfo;
    for (const auto& dependency : dependencies) {
        dependenciesFileInfo.append(QFileInfo(dependency));
    }
    return dependenciesFileInfo;
}

void MainWindow::onTbAddBinaryClicked() {
    const QString binaryPath = ui->le_binaryPath->text();
    addBinary(binaryPath, "bin", nullptr, false);
}

void MainWindow::onItemChanged(QStandardItem* item) {
    if (item->column() == ModelColumnFind) {
        // get the ModelIndex from the item
        const auto modelIndex = item->index();
        const auto nameIndex = modelIndex.siblingAtColumn(ModelColumnName);
        const auto pathIndex = modelIndex.siblingAtColumn(ModelColumnPath);
        // get the check state
        const auto checkState = item->checkState();
        const auto path = pathIndex.data().toString();

        if (checkState == Qt::Checked) {
            // If the item has been checked we should add as a child the dependencies
            const auto dependencies = getDependencies(path);
            qDebug() << "Dependencies of" << path << ":" << dependencies.count();
            // now each dependency is inserted as a child of the binary
            for (const auto& dependency : dependencies) {
                addBinary(dependency.filePath(), "lib", m_model->itemFromIndex(nameIndex), false);
            }
        }
        else {
            // If the item has been unchecked we should remove all the children
            auto* nameItem = m_model->itemFromIndex(nameIndex);
            nameItem->removeRows(0, nameItem->rowCount());
        }
    }
}

void
MainWindow::addBinary(const QString& binaryPath, const QString& destDir, QStandardItem* parentItem, bool findDeps) {
    QFileInfo binaryFileInfo(binaryPath);
    if (!binaryFileInfo.exists()) {
        return;
    }

    if (!parentItem) {
        parentItem = m_model->invisibleRootItem();
    }
    QList<QStandardItem*> items;
    // the name and the path are not editable
    auto* nameItem = new QStandardItem(binaryFileInfo.fileName());
    nameItem->setEditable(false);
    items.append(nameItem);
    auto* pathItem = new QStandardItem(binaryFileInfo.filePath());
    pathItem->setEditable(false);
    items.append(pathItem);
    // Create a checkbox to insert in the column 3
    auto* checkBox = new QStandardItem();
    checkBox->setCheckable(true);
    checkBox->setCheckState(findDeps ? Qt::Checked : Qt::Unchecked);
    items.append(checkBox);
    items.append(new QStandardItem(destDir));

    parentItem->appendRow(items);
}

QSet<QStandardItem*> MainWindow::getAllChild(QStandardItem* parentItem) const {
    // get all the childs of the parentItem recursively
    QSet<QStandardItem*> childs;
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        auto* child = parentItem->child(i);
        childs.insert(child);
        childs.unite(getAllChild(child));
    }
    return childs;
}

void MainWindow::onTbOutputPathFindClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Output Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
    );
    ui->le_outputPath->setText(dir);
}

void MainWindow::onTbFindBinaryClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    // allow hidden files
    dialog.setFilter(dialog.filter() | QDir::Hidden);

    QStringList files;
    if (dialog.exec()) {
        files = dialog.selectedFiles();
    }

    for (const auto& file : files) {
        addBinary(file, "bin", nullptr, false);
    }
}

void MainWindow::onPbInstallClicked() {
    // take the output path, check if the path exists, if not, create it and then copy the files
    const QString outputPath = ui->le_outputPath->text();
    QDir dir(outputPath);
    if (dir.exists(outputPath)) {
        // ask the user if he wants to overwrite the files
        // Create the message with the path information
        const QString message = "The output path already exists, do you want to overwrite it?\n" + outputPath;
        auto response = QMessageBox::question(this, "Overwrite", message);
        if (response == QMessageBox::No) {
            return;
        }
        // remove the directory
        dir.removeRecursively();
    }
    // create the directory
    {
        if (!dir.mkpath(outputPath)) {
            QMessageBox::critical(this, "Error", "Cannot create the output directory");
            return;
        }
    }
    installRecursive(ui->tw_organization->invisibleRootItem(), outputPath);

    QMessageBox::information(this, "Success", "The files have been installed successfully");
}

void MainWindow::installRecursive(QTreeWidgetItem* item, const QString& path) {
    // if the item has any child, we have to create the directory
    if (item->childCount() > 0) {
        QDir dir(QDir::cleanPath(path));
        const QString itemPath = item->text(0);
        if (!itemPath.isEmpty() && !dir.mkdir(itemPath)) {
            qWarning() << "Cannot create the directory " + path + "/" + item->text(0);
        }
        for (int i = 0; i < item->childCount(); ++i) {
            auto* child = item->child(i);
            if (!item->text(0).isEmpty()) {
                installRecursive(child, path + "/" + item->text(0));
            }
            else {
                installRecursive(child, path);
            }
        }
    }
    else {
        // install this item in the path
        QFile file(item->text(1));
        // Check first if the file exists, if not we will skip it
        if (!file.exists()) {
            qWarning() << "The file " + item->text(1) + " does not exist";
            return;
        }
        // Then check if the destination path exists, if exist we will skip it
        QFile destFile(path + "/" + item->text(0));
        if (destFile.exists()) {
            return;
        }
        const bool ok = file.copy(destFile.fileName());
        //const bool ok = QFile::copy(item->text(1), path + "/" + item->text(0));
        if (!ok) {
            qWarning() << "Cannot copy the file " + item->text(1) + " to " + path + "/" + item->text(0);
        }
    }
}

void MainWindow::onCbZippedToggled(bool checked) {
    // Check if the command "tar" exists
    if (!checked) {
        return;
    }
    QProcess process;
    process.setProgram("tar");
    process.setArguments({ "--version" });
    process.start();
    process.waitForFinished();
    const int exitCode = process.exitCode();
    if (exitCode != 0) {
        QMessageBox::critical(this, "Error", "The command tar is not installed");
        QSignalBlocker blocker(ui->cb_zipped);
        ui->cb_zipped->setChecked(false);
    }
}

void MainWindow::onCustomContextMenuRequestedPaths(const QPoint& pos) {
    const QModelIndex index = ui->tv_paths->indexAt(pos);
    if (!index.isValid()) {
        return;
    }
    // we have to check if there is a multi-selection
    const QModelIndexList selectedIndexes = ui->tv_paths->selectionModel()->selectedRows();
    QMenu menu(this);
    QAction* removeAction = nullptr;
    QAction* changeDestDirAction = nullptr;
    if (selectedIndexes.count() == 1) {
        removeAction = menu.addAction("Remove");
    }
    if (selectedIndexes.count() > 1) {
        changeDestDirAction = menu.addAction("Change destination directory");
        removeAction = menu.addAction("Remove");
    }
    QAction* action = menu.exec(ui->tv_paths->viewport()->mapToGlobal(pos));
    if (removeAction && action == removeAction) {
        // remove all the selected indexes
        QList<QPersistentModelIndex> persistentIndexes;
        for (const QModelIndex& idx : selectedIndexes) {
            persistentIndexes.append(QPersistentModelIndex(idx));
        }
        for (const auto& idx : persistentIndexes) {
            if (!idx.isValid()) {
                continue;
            }
            QStandardItem* item = m_model->itemFromIndex(idx);
            if (item->parent()) {
                item->parent()->removeRow(item->row());
            }
            else {
                m_model->removeRow(item->row());
            }
        }
    }
    else if (changeDestDirAction && action == changeDestDirAction) {
        RenamePathDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            const QString text = dialog.getText();
            for (const QModelIndex& idx : selectedIndexes) {
                const auto destIdx = idx.siblingAtColumn(ModelColumnDestination);
                m_model->setData(destIdx, text);
            }
        }
    }
}

void MainWindow::onPbAddFromFileSystemClicked() {
    // check if the tree view that uses the file system model has any selected item
    const QModelIndexList selectedIndexes = ui->tv_fileExplorer->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return;
    }
    // If the selected item is a directory, we will add all the files inside the directory
    for (const QModelIndex& idx : selectedIndexes) {
        if (m_fileSystemModel->isDir(idx)) {
            // get the path of the directory
            const QString path = m_fileSystemModel->filePath(idx);
            // get all the files inside the directory
            QDir dir(path);
            const auto files = dir.entryInfoList(QDir::Files);
            for (const auto& file : files) {
                addBinary(file.filePath(), "bin", nullptr, false);
            }
        }
        else {
            // if the selected item is a file, we will add only this file
            const QString path = m_fileSystemModel->filePath(idx);
            addBinary(path, "bin", nullptr, false);
        }
    }
}

void MainWindow::onPbAddFromFilesystemCustomDestClicked() {
    RenamePathDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const QString text = dialog.getText();
        if (text.isEmpty()) {
            QMessageBox::warning(this, "Warning", "The destination directory cannot be empty");
            return;
        }
        // check if the tree view that uses the file system model has any selected item
        const QModelIndexList selectedIndexes = ui->tv_fileExplorer->selectionModel()->selectedRows();
        if (selectedIndexes.isEmpty()) {
            return;
        }
        // If the selected item is a directory, we will add all the files inside the directory
        for (const QModelIndex& idx : selectedIndexes) {
            if (m_fileSystemModel->isDir(idx)) {
                // get the path of the directory
                const QString path = m_fileSystemModel->filePath(idx);
                // get all the files inside the directory
                QDir dir(path);
                const auto files = dir.entryInfoList(QDir::Files);
                for (const auto& file : files) {
                    addBinary(file.filePath(), text, nullptr, false);
                }
            }
            else {
                // if the selected item is a file, we will add only this file
                const QString path = m_fileSystemModel->filePath(idx);
                addBinary(path, text, nullptr, false);
            }
        }
    }
}

void MainWindow::changeThemeFlatGray() {
    //const QColor gunMetal = QColor(42, 52, 54);
    //const QColor dimGray = QColor(99, 110, 114);
    //const QColor x11Gray = QColor(178, 190, 195);
    const QColor platinum = QColor(229, 233, 237);

    const QColor slateGray = QColor(112, 128, 144);
    //const QColor lightGray = QColor(211, 211, 211);

    QColor highlightColor = slateGray;
    QColor highlightTextColor = platinum;

    auto dark_palette = QPalette();
    dark_palette.setColor(QPalette::Window, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::WindowText, Qt::white);
    dark_palette.setColor(QPalette::Base, QColor(35, 35, 35));
    dark_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
    dark_palette.setColor(QPalette::ToolTipText, Qt::white);
    dark_palette.setColor(QPalette::Text, Qt::white);
    dark_palette.setColor(QPalette::Button, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::ButtonText, Qt::white);
    dark_palette.setColor(QPalette::BrightText, Qt::red);
    dark_palette.setColor(QPalette::Link, QColor(42, 130, 218));
    dark_palette.setColor(QPalette::Highlight, highlightColor);
    dark_palette.setColor(QPalette::HighlightedText, highlightTextColor);
    dark_palette.setColor(QPalette::Active, QPalette::Button, QColor(53, 53, 53));
    dark_palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    dark_palette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGray);
    dark_palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    dark_palette.setColor(QPalette::Disabled, QPalette::Light, QColor(53, 53, 53));
    QApplication::setPalette(dark_palette);
}

void MainWindow::onActionSaveAsClicked() {
    // Open a dialog to select the file in a .bin format under home directory/.local/share/InstallerCreator
    // If the path does not exist, create it
    QDir dir(QDir::homePath() + "/.local/share/InstallerCreator");
    if (!dir.exists()) {
        if (!dir.mkpath(dir.path())) {
            QMessageBox::critical(this, "Error", "Cannot create the directory " + dir.path());
            return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save as", dir.path(), "Binary file (*.bin)");
    if (fileName.isEmpty()) {
        return;
    }
    // append the extension if it does not exist
    if (!fileName.endsWith(".bin")) {
        fileName.append(".bin");
    }


    // Convert the model data into a binary data
    QVector<BinaryData> binaryDataTopLevel;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto* item = m_model->item(i);
        BinaryData binaryData;
        saveRecursiveBinaryData(&binaryData, item);
        binaryDataTopLevel.append(binaryData);
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open the file " + fileName);
        return;
    }
    QDataStream out(&file);
    out << binaryDataTopLevel;
}

void MainWindow::onActionLoadClicked() {
    // Open a dialog to select the file in a .bin format under home directory/.local/share/InstallerCreator
    // If the path does not exist, create it
    QDir dir(QDir::homePath() + "/.local/share/InstallerCreator");
    if (!dir.exists()) {
        if (!dir.mkpath(dir.path())) {
            QMessageBox::critical(this, "Error", "Cannot create the directory " + dir.path());
            return;
        }
    }

    const QString fileName = QFileDialog::getOpenFileName(this, "Load", dir.path(), "Binary file (*.bin)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open the file " + fileName);
        return;
    }
    QDataStream in(&file);
    QVector<BinaryData> binaryDataTopLevel;
    in >> binaryDataTopLevel;

    // clear the model
    m_model->clear();
    m_model->setColumnCount(4);
    m_model->setHeaderData(ModelColumnName, Qt::Horizontal, "Name");
    m_model->setHeaderData(ModelColumnPath, Qt::Horizontal, "Path");
    m_model->setHeaderData(ModelColumnFind, Qt::Horizontal, "Install deps");
    m_model->setHeaderData(ModelColumnDestination, Qt::Horizontal, "Destination");

    // now we have to loop over the binaryDataTopLevel and add the items to the model using the loadRecursiveBinaryData
    for (const auto& binaryData : binaryDataTopLevel) {
        QStandardItem* parentItem = m_model->invisibleRootItem();
        loadRecursiveBinaryData(binaryData, parentItem);
    }
}

void MainWindow::saveRecursiveBinaryData(BinaryData* binaryData, QStandardItem* parentItem) {
    // get the data from the parent item
    const QModelIndex pathIndex = parentItem->index().siblingAtColumn(ModelColumnPath);
    const QModelIndex findIndex = parentItem->index().siblingAtColumn(ModelColumnFind);
    const QModelIndex destinationIndex = parentItem->index().siblingAtColumn(ModelColumnDestination);

    binaryData->setFileName(pathIndex.data().toString());
    // take the check state of the find index
    if (findIndex.isValid())
    {
        const auto item = m_model->itemFromIndex(findIndex);
        const bool findDeps = item->checkState() == Qt::Checked;
        binaryData->setFindDeps(findDeps);
    }
    binaryData->setDestination(destinationIndex.data().toString());

    // now we have to loop over the childs
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        auto* child = parentItem->child(i);
        BinaryData childBinaryData;
        saveRecursiveBinaryData(&childBinaryData, child);
        binaryData->appendDependency(childBinaryData);
    }
}

void MainWindow::loadRecursiveBinaryData(const BinaryData& binaryData, QStandardItem* parentItem) {
    // create the items
    QList<QStandardItem*> items;
    QFileInfo binaryFileInfo(binaryData.fileName());
    auto* nameItem = new QStandardItem(binaryFileInfo.fileName());
    nameItem->setEditable(false);
    items.append(nameItem);
    auto* pathItem = new QStandardItem(binaryFileInfo.filePath());
    pathItem->setEditable(false);
    items.append(pathItem);
    // Create a checkbox to insert in the column 3
    auto* checkBox = new QStandardItem();
    checkBox->setCheckable(true);
    checkBox->setCheckState(binaryData.findDeps() ? Qt::Checked : Qt::Unchecked);
    items.append(checkBox);
    items.append(new QStandardItem(binaryData.destination()));

    parentItem->appendRow(items);

    // now we have to loop over the dependencies
    for (const auto& dependency : binaryData.dependencies()) {
        loadRecursiveBinaryData(dependency, items[0]);
    }
}
