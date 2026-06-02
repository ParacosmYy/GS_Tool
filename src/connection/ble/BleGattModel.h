/**
 * @file BleGattModel.h
 * @brief GATT服务/特征树模型 — 以树形结构展示BLE GATT层级
 *
 * 职责: 管理GATT服务→特征→描述符的层级数据，
 * 提供QAbstractItemModel接口供QTreeView展示。
 */
#ifndef BLEGATTMODEL_H
#define BLEGATTMODEL_H

#include <QAbstractItemModel>
#include <QVariantList>

/**
 * @brief BLE GATT服务/特征树形模型
 *
 * 三层结构: 服务(Service) → 特征(Characteristic) → 描述符(Descriptor)。
 * 每层显示名称和UUID，支持动态更新。
 */
class BleGattModel : public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * @brief 构造GATT模型
     * @param parent 父对象
     */
    explicit BleGattModel(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BleGattModel() override;

    // ---- QAbstractItemModel 接口实现 ----

    /** @brief 模型索引对应的显示数据 */
    QVariant data(const QModelIndex& index, int role) const override;

    /** @brief 获取父项下子项数量 */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 列数(固定为1: 名称+UUID合并显示) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 获取子项的模型索引 */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    /** @brief 获取父项的模型索引 */
    QModelIndex parent(const QModelIndex& child) const override;

    // ---- 数据操作接口 ----

    /**
     * @brief 设置GATT服务数据
     * @param services 服务列表，每项包含name/uuid/characteristics
     */
    void setServices(const QVariantList& services);

    /**
     * @brief 获取服务数量
     * @return 当前加载的服务总数
     */
    int serviceCount() const;

private:
    /** @brief GATT服务数据列表 */
    QVariantList m_services;
};

#endif // BLEGATTMODEL_H
