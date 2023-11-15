//
// Created by pablo on 11/15/23.
//

#include "BinaryData.h"

BinaryData::BinaryData(QObject *parent) : QObject(parent) {

}

BinaryData::BinaryData(const BinaryData &other) {
    m_findDeps = other.m_findDeps;
    m_fileName = other.m_fileName;
    m_destination = other.m_destination;
    m_dependencies = other.m_dependencies;
}

BinaryData &BinaryData::operator=(const BinaryData &other) {
    m_findDeps = other.m_findDeps;
    m_fileName = other.m_fileName;
    m_destination = other.m_destination;
    m_dependencies = other.m_dependencies;
    return *this;
}

BinaryData::BinaryData(BinaryData &&other) noexcept {
    m_findDeps = other.m_findDeps;
    m_fileName = std::move(other.m_fileName);
    m_destination = std::move(other.m_destination);
    m_dependencies = std::move(other.m_dependencies);
}

BinaryData::~BinaryData() {

}

QDataStream &operator<<(QDataStream &out, const BinaryData &binaryData) {
    out << binaryData.m_findDeps;
    out << binaryData.m_fileName;
    out << binaryData.m_destination;
    out << binaryData.m_dependencies;
    return out;
}

QDataStream &operator>>(QDataStream &in, BinaryData &binaryData) {
    in >> binaryData.m_findDeps;
    in >> binaryData.m_fileName;
    in >> binaryData.m_destination;
    in >> binaryData.m_dependencies;
    return in;
}
