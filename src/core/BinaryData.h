//
// Created by pablo on 11/15/23.
//

#ifndef INSTALLERCREATOR_BINARYDATA_H
#define INSTALLERCREATOR_BINARYDATA_H

#include <QObject>

class BinaryData : public QObject {
Q_OBJECT
public:
    explicit BinaryData(QObject *parent = nullptr);

    BinaryData(const BinaryData &other);

    BinaryData &operator=(const BinaryData &other);

    BinaryData(BinaryData &&other) noexcept;

    ~BinaryData() override;

    friend QDataStream &operator<<(QDataStream &out, const BinaryData &binaryData);

    friend QDataStream &operator>>(QDataStream &in, BinaryData &binaryData);

    void setFindDeps(bool findDeps) {
        m_findDeps = findDeps;
    }

    bool findDeps() const {
        return m_findDeps;
    }

    void setFileName(const QString &fileName) {
        m_fileName = fileName;
    }

    QString fileName() const {
        return m_fileName;
    }

    void setDestination(const QString &destination) {
        m_destination = destination;
    }

    QString destination() const {
        return m_destination;
    }

    void setDependencies(const QVector<BinaryData> &dependencies) {
        m_dependencies = dependencies;
    }

    QVector<BinaryData> dependencies() const {
        return m_dependencies;
    }

    void appendDependency(const BinaryData &dependency) {
        m_dependencies.append(dependency);
    }

private:
    bool m_findDeps = false;
    QString m_fileName;
    QString m_destination;

    QVector<BinaryData> m_dependencies;
};


#endif //INSTALLERCREATOR_BINARYDATA_H
