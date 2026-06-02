/**
 * @file BleGattModel.cpp
 * @brief GATT服务/特征树模型实现
 */

#include "connection/ble/BleGattModel.h"

BleGattModel::BleGattModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

BleGattModel::~BleGattModel() = default;

QVariant BleGattModel::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return {}; // TODO: 按层级返回服务/特征/描述符数据
}

int BleGattModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 0; // TODO: 根据层级返回子项数
}

int BleGattModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1; // 单列: 名称+UUID
}

QModelIndex BleGattModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    return {}; // TODO: 创建模型索引
}

QModelIndex BleGattModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return {}; // TODO: 返回父索引
}

void BleGattModel::setServices(const QVariantList& services)
{
    beginResetModel();
    m_services = services;
    endResetModel();
}

int BleGattModel::serviceCount() const
{
    return m_services.size();
}
