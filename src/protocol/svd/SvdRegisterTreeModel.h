/**
 * @file SvdRegisterTreeModel.h
 * @brief SVD寄存器树模型 — 以树形结构展示MCU外设→寄存器→字段层级
 *
 * 职责: 将SVD设备数据解析为四层树(Device→Peripheral→Register→Field)，
 * 提供QAbstractItemModel接口供QTreeView展示。
 * 七列: 名称、地址、偏移、位宽、访问权限、复位值、描述。
 */
#ifndef SVD_REGISTER_TREE_MODEL_H
#define SVD_REGISTER_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QString>

struct SvdDevice;
struct SvdPeripheral;
struct SvdRegister;
struct SvdField;

/**
 * @brief SVD寄存器树内部节点
 *
 * 四层层级: Device → Peripheral → Register → Field。
 * 每个节点存储对应层级的属性数据，用于模型显示。
 */
struct SvdTreeNode {
    /** @brief 节点层级类型 */
    enum class Level {
        Device,     ///< 设备根节点
        Peripheral, ///< 外设节点
        Register,   ///< 寄存器节点
        Field       ///< 字段节点
    };

    QString name;                       ///< 节点显示名称
    QString address;                    ///< 地址(十六进制)
    QString offset;                     ///< 偏移(十六进制)
    QString size;                       ///< 位宽
    QString access;                     ///< 访问权限(read/write/read-write)
    QString resetValue;                 ///< 复位值(十六进制)
    QString description;                ///< 描述文本
    Level   level = Level::Device;      ///< 节点层级
    QList<SvdTreeNode*> children;       ///< 子节点列表
    SvdTreeNode* parent = nullptr;      ///< 父节点指针

    ~SvdTreeNode() { qDeleteAll(children); } ///< 递归删除子节点
};

/**
 * @brief SVD寄存器树形模型
 *
 * 四层结构: Device → Peripheral → Register → Field。
 * 七列显示: 名称、地址、偏移、位宽、访问权限、复位值、描述。
 * 支持setDevice()批量加载和统计追踪。
 */
class SvdRegisterTreeModel : public QAbstractItemModel {
    Q_OBJECT
    Q_DISABLE_COPY(SvdRegisterTreeModel)

public:
    /** @brief 模型列索引枚举 */
    enum Column {
        ColName = 0,        ///< 名称列
        ColAddress,         ///< 地址列
        ColOffset,          ///< 偏移列
        ColSize,            ///< 位宽列
        ColAccess,          ///< 访问权限列
        ColResetValue,      ///< 复位值列
        ColDescription,     ///< 描述列
        ColCount            ///< 列总数
    };

    /** @brief 构造SVD寄存器树模型 @param parent 父对象 */
    explicit SvdRegisterTreeModel(QObject* parent = nullptr);
    /** @brief 析构函数，释放根节点 */
    ~SvdRegisterTreeModel() override;

    // ---- QAbstractItemModel 接口 ----
    /** @brief 获取子项的模型索引 */
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 获取父项的模型索引 */
    QModelIndex parent(const QModelIndex& child) const override;
    /** @brief 获取父项下子项数量 */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 列数(固定7列) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 获取显示/工具提示数据 */
    QVariant data(const QModelIndex& index, int role) const override;
    /** @brief 获取表头数据 */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // ---- 数据操作接口 ----
    /** @brief 设置SVD设备数据并重建内部树 @param device SVD设备结构体 */
    void setDevice(const SvdDevice& device);
    /** @brief 清除所有树数据 */
    void clear();

    // ---- 统计接口 ----
    /** @brief 获取累计展开操作次数 @return 展开总次数 */
    quint64 totalExpandCount() const { return m_totalExpandCount; }
    /** @brief 获取累计折叠操作次数 @return 折叠总次数 */
    quint64 totalCollapseCount() const { return m_totalCollapseCount; }
    /** @brief 获取累计搜索操作次数 @return 搜索总次数 */
    quint64 totalSearchCount() const { return m_totalSearchCount; }
    /** @brief 获取累计加载设备次数 @return 设备加载总次数 */
    quint64 totalDeviceLoads() const { return m_totalDeviceLoads; }
    /** @brief 获取总节点数 @return 节点总数 */
    int totalNodeCount() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    /** @brief 从模型索引获取内部节点指针 @param index 模型索引 @return 节点指针 */
    SvdTreeNode* nodeFromIndex(const QModelIndex& index) const;

private:
    /** @brief 从SvdDevice构建完整树 @param device SVD设备数据 */
    void buildTree(const SvdDevice& device);
    /** @brief 添加外设节点 @param peripheral 外设数据 @param parent 父节点 */
    void addPeripheral(const SvdPeripheral& peripheral, SvdTreeNode* parent);
    /** @brief 添加寄存器节点 @param reg 寄存器数据 @param parent 父节点 */
    void addRegister(const SvdRegister& reg, SvdTreeNode* parent);
    /** @brief 递归统计节点总数 @param node 起始节点 @return 子孙节点总数(含自身) */
    int countNodes(const SvdTreeNode* node) const;
    /** @brief 格式化地址为十六进制字符串 @param addr 地址值 @return "0x..."格式字符串 */
    static QString formatAddress(quint64 addr);

    SvdTreeNode* m_rootNode;                ///< 虚拟根节点(不显示)
    quint64 m_totalExpandCount = 0;         ///< 累计展开操作次数
    quint64 m_totalCollapseCount = 0;       ///< 累计折叠操作次数
    quint64 m_totalSearchCount = 0;         ///< 累计搜索操作次数
    quint64 m_totalDeviceLoads = 0;         ///< 累计加载设备次数
};

#endif // SVD_REGISTER_TREE_MODEL_H
