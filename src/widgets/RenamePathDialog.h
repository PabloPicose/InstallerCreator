//
// Created by pablo on 11/9/23.
//

#ifndef INSTALLERCREATOR_RENAMEPATHDIALOG_H
#define INSTALLERCREATOR_RENAMEPATHDIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class RenamePathDialog; }
QT_END_NAMESPACE

class RenamePathDialog : public QDialog {
Q_OBJECT

public:
    explicit RenamePathDialog(QWidget *parent = nullptr);

    ~RenamePathDialog() override;

    QString getText() const;

private:
    Ui::RenamePathDialog *ui;
};


#endif //INSTALLERCREATOR_RENAMEPATHDIALOG_H
