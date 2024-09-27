#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class PathCheckFinderWgt; }
QT_END_NAMESPACE

class PathCheckFinderWgt : public QWidget
{
    Q_OBJECT
public:
    enum class PathType {
        File,
        Directory
    };
    explicit PathCheckFinderWgt(QWidget* parent = nullptr);

    ~PathCheckFinderWgt() override;

    void init(const QString& path, PathType type,
              const std::function<bool(const QString&)>& checker = nullptr);

private slots:
    void onTbFindClicked();

    void onLblPathEdited(const QString& text);

private:
    void createConnections();
private:
    Ui::PathCheckFinderWgt* ui = nullptr;

    std::function<bool(const QString&)> m_checker;
    PathType m_type = PathType::File;
};
