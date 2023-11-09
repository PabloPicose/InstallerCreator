//
// Created by pablo on 11/3/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
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
        auto *item = ui->tw_organization->takeTopLevelItem(i);
        delete item;
    }
    // columns: 1. Name, 2. Path

    const auto childs = getAllChilds(m_model->invisibleRootItem());
    for (const auto &item: childs) {
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

    qDebug() << "Destinations: " << destinations;
    for (const auto &dest: destinations) {
        // split into parts
        QStringList parts = dest.split('/');
        QTreeWidgetItem *parent = ui->tw_organization->invisibleRootItem();
        for (const QString part: parts) {
            // if the parent is null, we will create a root item
            if (!parent) {
                parent = new QTreeWidgetItem(ui->tw_organization);
                parent->setText(0, part);
                continue;
            }
            // if the parent is not null, we will search if the part is already a child of the parent
            bool found = false;
            for (int i = 0; i < parent->childCount(); ++i) {
                auto *child = parent->child(i);
                if (child->text(0) == part) {
                    parent = child;
                    found = true;
                    break;
                }
            }
            // if the part is not a child of the parent, we will create a new child
            if (!found) {
                auto *child = new QTreeWidgetItem(parent);
                child->setText(0, part);
                parent = child;
            }
        }
    }
    // now we will loop over the childs, we will take the path and the name, and we will add it to the tree widget.
    for (const auto child: childs) {
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
        QTreeWidgetItem *parent = ui->tw_organization->invisibleRootItem();

        for (const auto &pathPar: parts) {
            for (int i = 0; i < parent->childCount(); ++i) {
                auto *childSearch = parent->child(i);
                if (childSearch->text(0) == pathPar) {
                    parent = childSearch;
                    break;
                }
            }
        }
        auto *item = new QTreeWidgetItem(parent);
        item->setText(0, name);
        item->setText(1, path);
    }
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

    connect(ui->cb_zipped, &QCheckBox::toggled,
            this, &MainWindow::onCbZippedToggled);
}

QList<QFileInfo> MainWindow::getDependencies(const QString &binaryPath) {

    QStringList dependencies;

    // Use QProcess to run ldd command
    QProcess process;
    process.setProgram("ldd");
    process.setArguments({binaryPath});

    // Start the process
    process.start();
    process.waitForFinished();

    // Read the output
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    // Extract the library paths
    for (const QString &line: lines) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.length() > 2) {
            dependencies.append(parts[2]);
        }
    }


    QList<QFileInfo> dependenciesFileInfo;
    for (const auto &dependency: dependencies) {
        dependenciesFileInfo.append(QFileInfo(dependency));
    }
    return dependenciesFileInfo;
}

void MainWindow::onTbAddBinaryClicked() {
    const QString binaryPath = ui->le_binaryPath->text();
    addBinary(binaryPath, true, nullptr);
}

void MainWindow::onItemChanged(QStandardItem *item) {
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
            for (const auto &dependency: dependencies) {
                addBinary(dependency.filePath(), false, m_model->itemFromIndex(nameIndex));
            }
        } else {
            // If the item has been unchecked we should remove all the children
            auto *nameItem = m_model->itemFromIndex(nameIndex);
            nameItem->removeRows(0, nameItem->rowCount());
        }
    }
}

void MainWindow::addBinary(const QString &binaryPath, bool onBinDir, QStandardItem *parentItem) {
    QFileInfo binaryFileInfo(binaryPath);
    if (!binaryFileInfo.exists()) {
        return;
    }

    if (!parentItem) {
        parentItem = m_model->invisibleRootItem();
    }
    QList<QStandardItem *> items;
    // the name and the path are not editable
    auto *nameItem = new QStandardItem(binaryFileInfo.fileName());
    nameItem->setEditable(false);
    items.append(nameItem);
    auto *pathItem = new QStandardItem(binaryFileInfo.filePath());
    pathItem->setEditable(false);
    items.append(pathItem);
    // Create a checkbox to insert in the column 3
    auto *checkBox = new QStandardItem();
    checkBox->setCheckable(true);
    checkBox->setCheckState(Qt::Unchecked);
    items.append(checkBox);
    QString path = onBinDir ? "bin" : "lib";
    items.append(new QStandardItem(path));

    parentItem->appendRow(items);
}

QSet<QStandardItem *> MainWindow::getAllChilds(QStandardItem *parentItem) const {
    // get all the childs of the parentItem recursively
    QSet<QStandardItem *> childs;
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        auto *child = parentItem->child(i);
        childs.insert(child);
        childs.unite(getAllChilds(child));
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
    // allow multi selection
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Select Binaries"),
                                                      "/home"
    );
    for (const auto &file: files) {
        addBinary(file, true, nullptr);
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
        if (!dir.mkpath(outputPath) ) {
            QMessageBox::critical(this, "Error", "Cannot create the output directory");
            return;
        }
    }
    installRecursive(ui->tw_organization->invisibleRootItem(), outputPath);

    QMessageBox::information(this, "Success", "The files have been installed successfully");
}

void MainWindow::installRecursive(QTreeWidgetItem *item, const QString &path) {
    // if the item has any child, we have to create the directory
    if (item->childCount() > 0) {
        QDir dir(path);
        if (!dir.mkdir(item->text(0)) ) {
            qWarning() << "Cannot create the directory " + path + "/" + item->text(0);
        }
        for (int i = 0; i < item->childCount(); ++i) {
            auto *child = item->child(i);
            installRecursive(child, path + "/" + item->text(0));
        }
    } else {
        // install this item in the path
        const bool ok = QFile::copy(item->text(1), path + "/" + item->text(0));
        if (!ok)
        {
            //QMessageBox::critical(this, "Error", "Cannot copy the file " + item->text(1) + " to " + path + "/" + item->text(0));
            qWarning() << "Cannot copy the file " + item->text(1) + " to " + path + "/" + item->text(0);
        }
    }
}

void MainWindow::onCbZippedToggled(bool checked) {
    // Check if the command "tar" exists
    QProcess process;
    process.setProgram("tar");
    process.setArguments({"--version"});
    process.start();
    process.waitForFinished();
    const int exitCode = process.exitCode();
    if (exitCode != 0) {
        QMessageBox::critical(this, "Error", "The command tar is not installed");
        QSignalBlocker blocker(ui->cb_zipped);
        ui->cb_zipped->setChecked(false);
    }
}
