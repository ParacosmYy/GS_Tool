/**
 * @file MqttTopicModel.h
 * @brief MQTT主题树模型 — 以树形结构管理MQTT主题层级
 *
 * 职责: 管理MQTT主题的层级数据(按/分隔符构建树)，
 * 提供QAbstractItemModel接口供QTreeView展示。
 */
#ifndef MQTTTOPICMODEL_H
#define MQTTTOPICMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QList>

/**
 * @brief 主题树节点
 *
 * 内部数据结构，存储主题层级节点及其子节点。
 */
struct TopicNode {
    QString name;                    ///< 节点名称(单级)
    QString fullPath;                ///< 完整路径(从根到此节点)
    int qos = 0;                     ///< QoS等级
    QList<TopicNode*> children;      ///< 子节点列表
    TopicNode* parent = nullptr;     ///< 父节点指针

    /** @brief 析构时递归删除子节点 */
    ~TopicNode() { qDeleteAll(children); }
};

/**
 * @brief MQTT主题树形模型
 *
 * 将MQTT主题按层级分隔符"/"解析为树形结构，
 * 支持动态添加/移除主题。两列: 主题、QoS。
 */
class MqttTopicModel : public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * @brief 构造主题模型
     * @param parent 父对象
     */
    explicit MqttTopicModel(QObject* parent = nullptr);

    /** @brief 析构函数，清理树节点 */
    ~MqttTopicModel() override;

    // ---- QAbstractItemModel 接口实现 ----

    /** @brief 模型索引对应的显示数据 */
    QVariant data(const QModelIndex& index, int role) const override;

    /** @brief 表头数据 */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /** @brief 获取父项下子项数量 */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 列数(固定2列: 主题、QoS) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 获取子项的模型索引 */
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 获取父项的模型索引 */
    QModelIndex parent(const QModelIndex& child) const override;

    // ---- 数据操作接口 ----

    /**
     * @brief 添加主题到模型
     * @param topic MQTT主题字符串(如 "sensor/temperature/room1")
     * @param qos 服务质量等级
     */
    void addTopic(const QString& topic, int qos = 0);

    /**
     * @brief 从模型中移除主题
     * @param topic 要移除的主题
     */
    void removeTopic(const QString& topic);

    /**
     * @brief 获取所有已添加的主题列表
     * @return 主题字符串列表
     */
    QStringList topics() const;

private:
    /**
     * @brief 根据模型索引获取节点
     * @param index 模型索引
     * @return 对应的TopicNode指针
     */
    TopicNode* nodeFromIndex(const QModelIndex& index) const;

    /**
     * @brief 查找或创建子节点
     * @param parentNode 父节点
     * @param name 子节点名称
     * @return 已有或新建的子节点
     */
    TopicNode* findOrCreateChild(TopicNode* parentNode, const QString& name);

    /**
     * @brief 在树中查找指定完整路径的叶节点
     * @param root 搜索起始节点
     * @param fullPath 目标完整路径
     * @return 匹配的节点指针，未找到返回nullptr
     */
    TopicNode* findLeafNode(TopicNode* root, const QString& fullPath) const;

    /** @brief 根节点(虚拟根，不显示) */
    TopicNode* m_rootNode;

    /** @brief 扁平主题列表(用于快速查找) */
    QStringList m_topics;
};

#endif // MQTTTOPICMODEL_H
