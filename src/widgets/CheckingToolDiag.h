#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class CheckingToolDiag; }
QT_END_NAMESPACE

class CheckingToolDiag : public QDialog
{
    Q_OBJECT
public:

    explicit CheckingToolDiag(QWidget* parent = nullptr);

    ~CheckingToolDiag() override;

private:
    Ui::CheckingToolDiag* ui = nullptr;

};