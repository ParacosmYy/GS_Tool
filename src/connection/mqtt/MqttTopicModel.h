/**
 * @file MqttTopicModel.h
 * @brief MQTT主题树模型 — 按层级结构管理MQTT主题，提供完整QAbstractItemModel接口
 */
#ifndef MQTTTOPICMODEL_H
#define MQTTTOPICMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QList>

/** @brief 主题树节点 — 存储主题层级数据，每个节点对应路径的一级段 */
struct TopicNode {
    QString name;                    ///< 节点名称(单级路径段)
    QString fullPath;                ///< 完整路径(从根到此节点)
    int qos = 0;                     ///< QoS等级(0/1/2)
    QList<TopicNode*> children;      ///< 子节点列表
    TopicNode* parent = nullptr;     ///< 父节点指针
    ~TopicNode() { qDeleteAll(children); } ///< 递归删除子节点
};

/**
 * @brief MQTT主题树形模型
 *
 * 将MQTT主题按"/"解析为树形结构，支持增删改查、QoS编辑。
 * 两列: 主题名、QoS等级。完整实现QAbstractItemModel接口。
 */
class MqttTopicModel : public QAbstractItemModel {
    Q_OBJECT

public:
    /** @brief 构造MQTT主题树模型 @param parent 父对象指针 */
    explicit MqttTopicModel(QObject* parent = nullptr);
    /** @brief 析构函数，释放整棵主题树 */
    ~MqttTopicModel() override;

    // ---- QAbstractItemModel 完整接口 ----

    /** @brief 获取显示/工具提示/编辑数据 */
    QVariant data(const QModelIndex& index, int role) const override;
    /** @brief 设置数据(支持QoS编辑) */
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    /** @brief 表头数据(主题、QoS) */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    /** @brief 父项下子项数量 */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 列数(固定2列) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 创建子节点模型索引 */
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 获取父索引 */
    QModelIndex parent(const QModelIndex& child) const override;
    /** @brief 项标志(可选择/可编辑QoS/启用) */
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    /** @brief 是否有子项(优化展开性能) */
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

    // ---- 主题管理接口 ----

    /** @brief 添加主题，按'/'分割构建层级 @param topic 主题路径 @param qos QoS(0/1/2) */
    void addTopic(const QString& topic, int qos = 0);
    /** @brief 移除主题并重建树 @param topic 主题路径 */
    void removeTopic(const QString& topic);
    /** @brief 更新已有主题QoS @param topic 主题路径 @param qos 新QoS @return 是否成功 */
    bool updateTopic(const QString& topic, int qos);
    /** @brief 清除所有主题 */
    void clearTopics();
    /** @brief 主题是否存在 @param topic 主题路径 @return 是否存在 */
    bool hasTopic(const QString& topic) const;
    /** @brief 当前主题数量 @return 主题总数 */
    int topicCount() const;
    /** @brief 获取指定主题QoS @param topic 主题路径 @return QoS等级，不存在返回-1 */
    int topicQos(const QString& topic) const;
    /** @brief 获取所有主题列表 @return 主题路径字符串列表 */
    QStringList topics() const;
    /** @brief 根据主题路径查找模型索引 @param topic 完整路径 @return 第0列索引 */
    QModelIndex findTopicIndex(const QString& topic) const;

    // ---- 统计接口 ----
    /** @brief 获取累计添加次数 @return 添加总次数 */
    quint64 totalTopicsAdded() const;
    /** @brief 获取累计移除次数 @return 移除总次数 */
    quint64 totalTopicsRemoved() const;
    /** @brief 获取累计去重跳过次数 @return 去重总次数 */
    quint64 totalDuplicateSkips() const;
    /** @brief 获取累计QoS更新次数 @return QoS更新总次数 */
    quint64 totalQosUpdates() const;
    /** @brief 获取累计消息路由次数 @return 路由总次数 */
    quint64 totalMessagesRouted() const;
    /** @brief 获取树节点总数(含非叶节点) @return 节点总数(不含虚拟根节点) */
    int totalNodeCount() const;
    /** @brief 重置统计计数器 */
    void resetTopicStatistics();

    // ---- 消息路由接口 ----
    /**
     * @brief 将收到的消息路由到匹配的主题节点，更新统计
     * @param topic 消息主题
     * @param payload 消息负载(用于更新节点数据)
     * @return true=匹配到至少一个主题
     */
    bool routeMessage(const QString& topic, const QByteArray& payload);

signals:
    /** @brief 主题添加信号 @param topic 主题路径 @param qos QoS等级 */
    void topicAdded(const QString& topic, int qos);
    /** @brief 主题移除信号 @param topic 主题路径 */
    void topicRemoved(const QString& topic);
    /** @brief QoS变更信号 @param topic 主题路径 @param oldQos 旧QoS @param newQos 新QoS */
    void topicQosChanged(const QString& topic, int oldQos, int newQos);
    /** @brief 所有主题清除信号 */
    void topicsCleared();

private:
    /** @brief 从模型索引获取对应的树节点指针 @param index 模型索引 @return 对应的TopicNode指针 */
    TopicNode* nodeFromIndex(const QModelIndex& index) const;
    /** @brief 在父节点下查找或创建指定名称的子节点 @param parentNode 父节点 @param name 子节点名称 @return 已有或新创建的子节点指针 */
    TopicNode* findOrCreateChild(TopicNode* parentNode, const QString& name);
    /** @brief 递归查找指定完整路径的叶节点 @param root 搜索起始根节点 @param fullPath 目标完整路径 @return 匹配的叶节点指针，未找到返回nullptr */
    TopicNode* findLeafNode(TopicNode* root, const QString& fullPath) const;
    /** @brief 递归统计以指定节点为根的子树节点总数 @param node 起始节点 @return 节点总数(包含自身) */
    int countNodes(const TopicNode* node) const;
    /** @brief MQTT通配符匹配(支持+和#) @param topicParts 消息主题段 @param subParts 订阅模式段 @return 是否匹配 */
    bool topicMatchesSubscription(const QStringList& topicParts, const QStringList& subParts) const;

    TopicNode* m_rootNode;                ///< 根节点(虚拟，不显示)
    QStringList m_topics;                 ///< 扁平主题列表(快速查找)

    quint64 m_totalTopicsAdded = 0;       ///< 累计添加
    quint64 m_totalTopicsRemoved = 0;     ///< 累计移除
    quint64 m_totalDuplicateSkips = 0;    ///< 累计去重
    quint64 m_totalQosUpdates = 0;        ///< 累计QoS更新
    quint64 m_totalMessagesRouted = 0;    ///< 累计消息路由次数
};

#endif // MQTTTOPICMODEL_H
