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

/**
 * @brief MQTT主题树形模型
 *
 * 将MQTT主题按层级分隔符"/"解析为树形结构，
 * 支持动态添加/移除主题。
 */
class MqttTopicModel : public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * @brief 构造主题模型
     * @param parent 父对象
     */
    explicit MqttTopicModel(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~MqttTopicModel() override;

    // ---- QAbstractItemModel 接口实现 ----

    /** @brief 模型索引对应的显示数据 */
    QVariant data(const QModelIndex& index, int role) const override;

    /** @brief 获取父项下子项数量 */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 列数(固定1列) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 获取子项的模型索引 */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 获取父项的模型索引 */
    QModelIndex parent(const QModelIndex& child) const override;

    // ---- 数据操作接口 ----

    /**
     * @brief 添加主题到模型
     * @param topic MQTT主题字符串(如 "sensor/temperature/room1")
     */
    void addTopic(const QString& topic);

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
    /** @brief 扁平主题列表(用于快速查找) */
    QStringList m_topics;
};

#endif // MQTTTOPICMODEL_H
