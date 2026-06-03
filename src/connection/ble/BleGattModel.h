/**
 * @file BleGattModel.h
 * @brief GATT服务/特征树模型 — 以树形结构展示BLE GATT层级
 *
 * 职责: 管理GATT服务→特征→描述符的层级数据，
 * 提供QAbstractItemModel接口供QTreeView展示。
 * 四列: 名称、UUID、属性、值。
 */
#ifndef BLEGATTMODEL_H
#define BLEGATTMODEL_H

#include <QAbstractItemModel>
#include <QColor>
#include <QVariantList>
#include <QList>
#include <QStringList>

/**
 * @brief GATT树节点内部数据结构
 * 三层层级: Service → Characteristic → Descriptor
 */
struct GattNode {
    QString name;               ///< 节点显示名称
    QString uuid;               ///< 节点UUID
    QString properties;         ///< 属性描述(如 READ|WRITE|NOTIFY)
    QString value;              ///< 当前值(十六进制表示)
    QList<GattNode*> children;  ///< 子节点列表
    GattNode* parent = nullptr; ///< 父节点指针
    ~GattNode() { qDeleteAll(children); } ///< 析构时递归删除所有子节点
};

/**
 * @brief BLE GATT服务/特征树形模型
 * 三层结构: Service → Characteristic → Descriptor。
 * 四列显示: 名称、UUID、属性、值。
 * 支持setServices()批量更新和updateValueByUuid()单项更新。
 */
class BleGattModel : public QAbstractItemModel {
    Q_OBJECT
    Q_DISABLE_COPY(BleGattModel)

public:
    /** @brief 模型列索引枚举 */
    enum Column {
        ColName = 0,    ///< 名称列
        ColUuid,        ///< UUID列
        ColProperties,  ///< 属性列
        ColValue,       ///< 值列
        ColCount        ///< 列总数
    };

    /** @brief 构造GATT模型 @param parent 父对象 */
    explicit BleGattModel(QObject* parent = nullptr);
    /** @brief 析构函数，释放根节点 */
    ~BleGattModel() override;

    // ---- QAbstractItemModel 接口 ----
    /** @brief 获取模型索引对应的显示数据 */
    QVariant data(const QModelIndex& index, int role) const override;
    /** @brief 设置指定索引的数据(支持编辑值列) */
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    /** @brief 获取表头数据 */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    /** @brief 获取父项下子项数量 */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 列数(固定4列) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 获取子项的模型索引 */
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 获取父项的模型索引 */
    QModelIndex parent(const QModelIndex& child) const override;
    /** @brief 获取项标志(可选中/启用，值列可编辑) */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // ---- 数据操作接口 ----
    /** @brief 设置GATT服务数据并重建内部树 @param services 服务列表 */
    void setServices(const QVariantList& services);
    /** @brief 获取当前已加载的GATT服务数量 */
    int serviceCount() const;
    /** @brief 从模型索引获取内部GattNode指针 @param index 模型索引 @return GattNode指针 */
    GattNode* nodeFromIndex(const QModelIndex& index) const;
    /** @brief 根据UUID查找节点 @param uuid 目标UUID @return 匹配节点指针或nullptr */
    GattNode* findNodeByUuid(const QString& uuid) const;
    /** @brief 根据UUID更新节点值并通知视图 @param uuid 目标UUID @param newValue 新值 @return 成功返回true */
    bool updateValueByUuid(const QString& uuid, const QString& newValue);
    /** @brief 清除所有GATT服务数据 */
    void clear();
    /** @brief 获取所有服务名称列表 @return 服务名称QStringList */
    QStringList serviceNames() const;
    /** @brief 计算所有节点总数(服务+特征+描述符) @return 节点总数 */
    int totalNodeCount() const;

    // ---- 统计信息接口 ----
    /** @brief 获取总发现服务数 */
    quint64 totalServicesDiscovered() const { return m_totalServicesDiscovered; }
    /** @brief 获取总特征读取次数 */
    quint64 totalCharacteristicsRead() const { return m_totalCharacteristicsRead; }
    /** @brief 获取总写入次数 */
    quint64 totalWrites() const { return m_totalWrites; }
    /** @brief 获取错误计数 */
    quint64 errorCount() const { return m_errorCount; }
    /** @brief 重置所有GATT统计计数器 */
    void resetGattStatistics();

private:
    /** @brief 从QVariantMap构建子树 @param itemMap 数据映射 @param parentNode 父节点 */
    void buildNode(const QVariantMap& itemMap, GattNode* parentNode);
    /** @brief 递归查找UUID匹配节点 @param node 起点 @param uuid 目标UUID @return 匹配节点或nullptr */
    GattNode* findNodeRecursive(GattNode* node, const QString& uuid) const;
    /** @brief 递归统计节点总数 @param node 起点 @return 子孙节点总数(不含自身) */
    int countNodes(const GattNode* node) const;
    /** @brief 根据GattNode指针反向查找模型索引 @param node 目标节点 @return QModelIndex */
    QModelIndex indexForNode(GattNode* node) const;

    GattNode* m_rootNode;                          ///< 虚拟根节点(不显示)
    quint64 m_totalServicesDiscovered = 0;         ///< 总发现服务数
    quint64 m_totalCharacteristicsRead = 0;        ///< 总特征读取次数
    quint64 m_totalWrites = 0;                     ///< 总写入次数
    quint64 m_errorCount = 0;                      ///< 错误计数
};

#endif // BLEGATTMODEL_H
