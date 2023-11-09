//
// Created by pablo on 11/9/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_RenamePathDialog.h" resolved

#include "RenamePathDialog.h"
#include "ui_RenamePathDialog.h"


RenamePathDialog::RenamePathDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::RenamePathDialog) {
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

RenamePathDialog::~RenamePathDialog() {
    delete ui;
}

QString RenamePathDialog::getText() const {
    return ui->le_path->text();
}
