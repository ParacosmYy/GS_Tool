/**
 * @file MqttTopicModelHelpers.cpp
 * @brief MQTT主题树模型 — 内部辅助方法和统计接口实现
 *
 * 从MqttTopicModel.cpp拆分，负责:
 *   1. 节点查找/创建辅助方法(nodeFromIndex/findOrCreateChild/findLeafNode/countNodes)
 *   2. MQTT通配符匹配(topicMatchesSubscription)
 *   3. 统计计数器查询接口(totalTopicsAdded/Removed/DuplicateSkips/QosUpdates/NodeCount)
 *   4. 统计计数器重置(resetTopicStatistics)
 */

#include "connection/mqtt/MqttTopicModel.h"

// ============================================================
// 内部辅助方法
// ============================================================

/** @brief 从模型索引获取对应的树节点指针
 *  @param index 模型索引
 *  @return 对应的TopicNode指针
 */
TopicNode* MqttTopicModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid()
        ? static_cast<TopicNode*>(index.internalPointer())
        : m_rootNode;
}

/** @brief 在父节点下查找或创建指定名称的子节点
 *  @param parentNode 父节点指针
 *  @param name 子节点名称
 *  @return 已有或新创建的子节点指针
 */
TopicNode* MqttTopicModel::findOrCreateChild(TopicNode* parentNode,
                                              const QString& name)
{
    /* 查找已有子节点 */
    for (auto* child : parentNode->children) {
        if (child->name == name) return child;
    }

    /* 创建新子节点 */
    auto* newNode = new TopicNode;
    newNode->name = name;
    newNode->parent = parentNode;
    parentNode->children.append(newNode);
    return newNode;
}

/** @brief 递归查找指定完整路径的叶节点
 *  @param root 搜索起始根节点
 *  @param fullPath 目标完整路径
 *  @return 匹配的叶节点指针，未找到返回nullptr
 */
TopicNode* MqttTopicModel::findLeafNode(TopicNode* root, const QString& fullPath) const
{
    if (!root) return nullptr;

    /* 叶节点匹配完整路径 */
    if (root->fullPath == fullPath) return root;

    /* 递归搜索子节点 */
    for (auto* child : root->children) {
        TopicNode* found = findLeafNode(child, fullPath);
        if (found) return found;
    }
    return nullptr;
}

/** @brief 递归统计以指定节点为根的子树节点总数
 *  @param node 起始节点
 *  @return 该节点及其所有后代的总数(包含自身)
 */
int MqttTopicModel::countNodes(const TopicNode* node) const
{
    if (!node) return 0;
    int count = 1; /* 包含自身 */
    for (const auto* child : node->children) {
        count += countNodes(child);
    }
    return count;
}

/** @brief MQTT通配符匹配算法，支持+和#通配符
 *  @param topicParts 消息主题按'/'分割后的段列表
 *  @param subParts 订阅模式按'/'分割后的段列表
 *  @return true=订阅模式匹配该消息主题
 *
 *  规则:
 *  - '+' 匹配单一级段
 *  - '#' 匹配剩余所有级段(必须出现在最后)
 *  - 精确匹配其他字符
 */
bool MqttTopicModel::topicMatchesSubscription(const QStringList& topicParts,
                                               const QStringList& subParts) const
{
    int ti = 0, si = 0;
    while (si < subParts.size() && ti < topicParts.size()) {
        const QString& subPart = subParts.at(si);
        if (subPart == QStringLiteral("#")) {
            return true; /* '#'匹配剩余所有 */
        }
        if (subPart == QStringLiteral("+")) {
            /* '+'匹配任意单级 */
            ++ti;
            ++si;
            continue;
        }
        if (subPart != topicParts.at(ti)) {
            return false; /* 精确不匹配 */
        }
        ++ti;
        ++si;
    }

    /* 订阅末尾的#匹配剩余 */
    if (si < subParts.size() && subParts.at(si) == QStringLiteral("#")) {
        return true;
    }

    /* 两者必须同时耗尽 */
    return (ti == topicParts.size() && si == subParts.size());
}

// ============================================================
// 统计接口
// ============================================================

/** @brief 获取已添加主题的总数 @return 累计添加次数 */
quint64 MqttTopicModel::totalTopicsAdded() const
{
    return m_totalTopicsAdded;
}

/** @brief 获取已移除主题的总数 @return 累计移除次数 */
quint64 MqttTopicModel::totalTopicsRemoved() const
{
    return m_totalTopicsRemoved;
}

/** @brief 获取跳过重复主题的总次数 @return 累计去重跳过次数 */
quint64 MqttTopicModel::totalDuplicateSkips() const
{
    return m_totalDuplicateSkips;
}

/** @brief 获取QoS更新总次数 @return 累计更新次数 */
quint64 MqttTopicModel::totalQosUpdates() const
{
    return m_totalQosUpdates;
}

/** @brief 获取累计消息路由次数 @return 路由计数 */
quint64 MqttTopicModel::totalMessagesRouted() const
{
    return m_totalMessagesRouted;
}

/** @brief 获取树中所有节点总数(含非叶节点)
 *  @return 节点总数(不含虚拟根节点)
 */
int MqttTopicModel::totalNodeCount() const
{
    return countNodes(m_rootNode) - 1; /* 减去虚拟根节点 */
}

/** @brief 重置所有主题统计计数器 */
void MqttTopicModel::resetTopicStatistics()
{
    m_totalTopicsAdded = 0;
    m_totalTopicsRemoved = 0;
    m_totalDuplicateSkips = 0;
    m_totalQosUpdates = 0;
    m_totalMessagesRouted = 0;
}
