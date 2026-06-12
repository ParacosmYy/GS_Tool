/**
 * @file SvdRegisterTreeModel.cpp
 * @brief SVD寄存器树模型实现 — 构造/析构/树构建/QAbstractItemModel接口
 *
 * 包含构造/析构、setDevice/clear/buildTree、QAbstractItemModel核心接口、
 * 节点查找和地址格式化辅助方法。
 * 统计重置在 SvdRegisterTreeModelStats.cpp（由SvdViewerWidgetStats.cpp复用）。
 */

#include "protocol/svd/SvdRegisterTreeModel.h"
#include "protocol/svd/SvdTypes.h"

#include <QStringList>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造SVD寄存器树模型，创建虚拟根节点 @param parent 父QObject指针 */
SvdRegisterTreeModel::SvdRegisterTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(new SvdTreeNode())
{
    m_rootNode->name = QStringLiteral("SVD Root");
}

/** @brief 析构模型，释放根节点及其所有子节点 */
SvdRegisterTreeModel::~SvdRegisterTreeModel()
{
    delete m_rootNode;
}

// ============================================================================
// QAbstractItemModel 核心接口
// ============================================================================

/** @brief 获取子项的模型索引 @param row 行号 @param column 列号 @param parent 父索引 */
QModelIndex SvdRegisterTreeModel::index(int row, int column,
                                         const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || column >= ColCount) {
        return QModelIndex();
    }

    SvdTreeNode* parentNode = nodeFromIndex(parent);
    if (!parentNode || row >= parentNode->children.size()) {
        return QModelIndex();
    }

    SvdTreeNode* childNode = parentNode->children.at(row);
    return createIndex(row, column, childNode);
}

/** @brief 获取父项的模型索引 @param child 子项索引 */
QModelIndex SvdRegisterTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    auto* childNode = static_cast<SvdTreeNode*>(child.internalPointer());
    if (!childNode || !childNode->parent) {
        return QModelIndex();
    }

    SvdTreeNode* parentNode = childNode->parent;
    /* 根节点的父项为无效索引 */
    if (parentNode == m_rootNode) {
        return QModelIndex();
    }

    /* 查找父节点在祖父节点中的行号 */
    SvdTreeNode* grandParent = parentNode->parent;
    if (!grandParent) {
        return QModelIndex();
    }

    int row = grandParent->children.indexOf(parentNode);
    if (row < 0) {
        return QModelIndex();
    }

    return createIndex(row, 0, parentNode);
}

/** @brief 获取父项下子项数量 @param parent 父索引 */
int SvdRegisterTreeModel::rowCount(const QModelIndex& parent) const
{
    SvdTreeNode* node = nodeFromIndex(parent);
    return node ? node->children.size() : 0;
}

/** @brief 列数(固定7列) @param parent 父索引(未使用) */
int SvdRegisterTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColCount;
}

/** @brief 获取模型索引对应的显示/工具提示数据 @param index 模型索引 @param role 显示角色 */
QVariant SvdRegisterTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    auto* node = static_cast<SvdTreeNode*>(index.internalPointer());
    if (!node) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColName:        return node->name;
        case ColAddress:     return node->address;
        case ColOffset:      return node->offset;
        case ColSize:        return node->size;
        case ColAccess:      return node->access;
        case ColResetValue:  return node->resetValue;
        case ColDescription: return node->description.left(80);
        default:             return QVariant();
        }
    }

    if (role == Qt::ToolTipRole) {
        /* 工具提示: 显示节点完整信息 */
        QStringList tip;
        tip << tr("<b>%1</b>").arg(node->name);
        if (!node->address.isEmpty()) {
            tip << tr("地址: %1").arg(node->address);
        }
        if (!node->offset.isEmpty()) {
            tip << tr("偏移: %1").arg(node->offset);
        }
        if (!node->size.isEmpty()) {
            tip << tr("位宽: %1").arg(node->size);
        }
        if (!node->access.isEmpty()) {
            tip << tr("访问: %1").arg(node->access);
        }
        if (!node->resetValue.isEmpty()) {
            tip << tr("复位值: %1").arg(node->resetValue);
        }
        if (!node->description.isEmpty()) {
            tip << tr("描述: %1").arg(node->description);
        }
        return tip.join(QStringLiteral("<br>"));
    }

    if (role == Qt::UserRole) {
        /* UserRole: 返回节点层级，便于委托/代理判断类型 */
        return static_cast<int>(node->level);
    }

    return QVariant();
}

/** @brief 获取表头数据 @param section 列号 @param orientation 方向 @param role 显示角色 */
QVariant SvdRegisterTreeModel::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case ColName:        return tr("名称");
    case ColAddress:     return tr("地址");
    case ColOffset:      return tr("偏移");
    case ColSize:        return tr("位宽");
    case ColAccess:      return tr("访问");
    case ColResetValue:  return tr("复位值");
    case ColDescription: return tr("描述");
    default:             return QVariant();
    }
}

// ============================================================================
// 数据操作接口
// ============================================================================

/** @brief 设置SVD设备数据并重建整个树模型 @param device SVD设备结构体 */
void SvdRegisterTreeModel::setDevice(const SvdDevice& device)
{
    beginResetModel();
    ++m_totalDeviceLoads;

    /* 清除旧数据 */
    qDeleteAll(m_rootNode->children);
    m_rootNode->children.clear();

    /* 构建新树 */
    buildTree(device);

    endResetModel();
}

/** @brief 清除所有树数据并重置模型 */
void SvdRegisterTreeModel::clear()
{
    beginResetModel();
    qDeleteAll(m_rootNode->children);
    m_rootNode->children.clear();
    endResetModel();
}

// ============================================================================
// 节点查找
// ============================================================================

/** @brief 从模型索引获取内部节点指针 @param index 模型索引 @return 节点指针，无效索引返回根节点 */
SvdTreeNode* SvdRegisterTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (index.isValid()) {
        auto* node = static_cast<SvdTreeNode*>(index.internalPointer());
        if (node) {
            return node;
        }
    }
    return m_rootNode;
}

/** @brief 递归统计以指定节点为根的子树节点总数 @param node 起始节点 @return 节点总数(包含自身) */
int SvdRegisterTreeModel::countNodes(const SvdTreeNode* node) const
{
    if (!node) { return 0; }
    int count = 1;
    for (const SvdTreeNode* child : node->children) {
        count += countNodes(child);
    }
    return count;
}

/** @brief 获取总节点数(不含虚拟根节点) @return 节点总数 */
int SvdRegisterTreeModel::totalNodeCount() const
{
    int count = 0;
    for (const SvdTreeNode* child : m_rootNode->children) {
        count += countNodes(child);
    }
    return count;
}

// ============================================================================
// 私有辅助: 树构建
// ============================================================================

/** @brief 从SvdDevice构建完整四层树 @param device SVD设备数据 */
void SvdRegisterTreeModel::buildTree(const SvdDevice& device)
{
    /* 创建设备根节点 */
    auto* devNode = new SvdTreeNode();
    devNode->name = device.name;
    devNode->description = device.description;
    devNode->level = SvdTreeNode::Level::Device;
    devNode->parent = m_rootNode;
    m_rootNode->children.append(devNode);

    /* 遍历外设 */
    for (const SvdPeripheral& peri : device.peripherals) {
        addPeripheral(peri, devNode);
    }
}

/** @brief 添加外设节点及其子寄存器 @param peripheral 外设数据 @param parent 父节点 */
void SvdRegisterTreeModel::addPeripheral(const SvdPeripheral& peripheral,
                                          SvdTreeNode* parent)
{
    auto* node = new SvdTreeNode();
    node->name = peripheral.name;
    node->address = formatAddress(peripheral.baseAddress);
    node->description = peripheral.description;
    node->level = SvdTreeNode::Level::Peripheral;
    node->parent = parent;
    parent->children.append(node);

    /* 遍历寄存器 */
    for (const SvdRegister& reg : peripheral.registers) {
        addRegister(reg, node);
    }
}

/** @brief 添加寄存器节点及其子字段 @param reg 寄存器数据 @param parent 父节点 */
void SvdRegisterTreeModel::addRegister(const SvdRegister& reg,
                                        SvdTreeNode* parent)
{
    auto* node = new SvdTreeNode();
    node->name = reg.name;
    node->address = formatAddress(reg.addressOffset);
    node->offset = formatAddress(reg.addressOffset);
    node->size = QString::number(reg.size);
    node->access = reg.access;
    node->resetValue = formatAddress(reg.resetValue);
    node->description = reg.description;
    node->level = SvdTreeNode::Level::Register;
    node->parent = parent;
    parent->children.append(node);

    /* 遍历字段 */
    for (const SvdField& field : reg.fields) {
        auto* fieldNode = new SvdTreeNode();
        fieldNode->name = field.name;
        fieldNode->offset = tr("bit %1").arg(field.bitOffset);
        fieldNode->size = QString::number(field.bitWidth);
        fieldNode->access = field.access;
        fieldNode->description = field.description;
        fieldNode->level = SvdTreeNode::Level::Field;
        fieldNode->parent = node;
        node->children.append(fieldNode);
    }
}

/** @brief 格式化地址为十六进制字符串 @param addr 地址值 @return "0x..."格式字符串 */
QString SvdRegisterTreeModel::formatAddress(quint64 addr)
{
    if (addr == 0) {
        return QStringLiteral("0x00000000");
    }
    return QStringLiteral("0x%1")
        .arg(addr, 8, 16, QChar('0'))
        .toUpper();
}

// 统计重置见 SvdViewerWidgetStats.cpp（复用 resetStatistics）
