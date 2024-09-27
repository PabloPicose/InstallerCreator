#include "PathCheckFinderWgt.h"
#include "ui_PathCheckFinderWgt.h"

#include <QFileDialog>

PathCheckFinderWgt::PathCheckFinderWgt(QWidget* parent) : QWidget(parent)
{
    ui = new Ui::PathCheckFinderWgt();
    ui->setupUi(this);
}

PathCheckFinderWgt::~PathCheckFinderWgt() {
    delete ui;
}

void PathCheckFinderWgt::init(const QString& path, PathType type,
                              const std::function<bool(const QString&)>& checker) {
    m_type = type;
    m_checker = checker;
    ui->le_path->setText(path);
    onLblPathEdited(path);
    createConnections();
}

void PathCheckFinderWgt::onLblPathEdited(const QString& text)
{
    bool isOk = !text.isEmpty();
    if (isOk && m_checker && !m_checker(text)) {
        isOk = false;
    }
    // Check that exists
    if (isOk) {
        if (m_type == PathType::File) {
            QFileInfo fileInfo(text);
            isOk = fileInfo.exists() && fileInfo.isFile();
        }
        else {
            isOk = QDir(text).exists();
        }
    }
    // Select the right icon
    if (isOk) {
        ui->lbl_check_icon->setPixmap(QPixmap(":/icons/small/check.png"));
    }
    else {
        ui->lbl_check_icon->setPixmap(QPixmap(":/icons/small/cross.png"));
    }
}

void PathCheckFinderWgt::createConnections()
{
    connect(ui->tb_find, &QToolButton::clicked, this, &PathCheckFinderWgt::onTbFindClicked);
    connect(ui->le_path, &QLineEdit::textChanged, this, &PathCheckFinderWgt::onLblPathEdited);
}

void PathCheckFinderWgt::onTbFindClicked()
{
    QString path;
    if (m_type == PathType::File) {
        path = QFileDialog::getOpenFileName(this, tr("Select File"));
    }
    else {
        path = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
    }
    if (!path.isEmpty()) {
        ui->le_path->setText(path);
    }
}
