/**
 * @file MqttTopicModel.cpp
 * @brief MQTT主题树模型实现
 */

#include "connection/mqtt/MqttTopicModel.h"

MqttTopicModel::MqttTopicModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

MqttTopicModel::~MqttTopicModel() = default;

QVariant MqttTopicModel::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return {}; // TODO: 返回主题层级数据
}

int MqttTopicModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 0; // TODO: 按层级返回子项数
}

int MqttTopicModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex MqttTopicModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    return {};
}

QModelIndex MqttTopicModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return {};
}

void MqttTopicModel::addTopic(const QString& topic)
{
    if (m_topics.contains(topic)) return;
    beginResetModel();
    m_topics.append(topic);
    endResetModel();
}

void MqttTopicModel::removeTopic(const QString& topic)
{
    if (!m_topics.contains(topic)) return;
    beginResetModel();
    m_topics.removeAll(topic);
    endResetModel();
}

QStringList MqttTopicModel::topics() const
{
    return m_topics;
}
