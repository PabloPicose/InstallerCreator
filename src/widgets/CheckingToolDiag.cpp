#include "CheckingToolDiag.h"
#include "ui_CheckingToolDiag.h"

CheckingToolDiag::CheckingToolDiag(QWidget* parent)
    : QDialog(parent), ui(new Ui::CheckingToolDiag) {
    ui->setupUi(this);

    ui->ldd_path_check_finder_wgt->init("/usr/bin/ldd", PathCheckFinderWgt::PathType::File);
    ui->tar_path_check_finder_wgt->init("/bin/tar", PathCheckFinderWgt::PathType::File);
}

CheckingToolDiag::~CheckingToolDiag() {}
