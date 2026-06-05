/**
 * @file BleScanTypes.h
 * @brief BLE扫描器公共类型定义 — 设备类型枚举与设备信息结构体
 *
 * 职责: 定义 BleDeviceType 枚举和 BleDeviceInfo 结构体，
 * 供 BleDeviceScanner 和 BleScanWidget 共享使用。
 */

#ifndef BLESCANTYPES_H
#define BLESCANTYPES_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMetaType>

/**
 * @brief BLE设备角色分类
 */
enum class BleDeviceType {
    Unknown,   ///< 未知角色
    Peripheral, ///< 外围设备（传感器、信标等）
    Central    ///< 中心设备（手机、网关等）
};

/**
 * @brief BLE设备发现信息
 *
 * 包含设备名称、地址、信号强度、已广播服务UUID、角色和最近发现时间。
 */
struct BleDeviceInfo {
    QString     name;          ///< 设备名称（可能为空）
    QString     address;       ///< 设备MAC地址（XX:XX:XX:XX:XX:XX）
    int         rssi = 0;      ///< 信号强度(dBm)，范围 -128~0
    QStringList serviceUuids;  ///< 已广播服务UUID列表
    BleDeviceType type = BleDeviceType::Unknown; ///< 设备角色
    QDateTime   lastSeen;      ///< 最近一次被发现的时间

    /** @brief 判断设备信息是否有效（地址非空） */
    bool isValid() const noexcept { return !address.isEmpty(); }
};

Q_DECLARE_METATYPE(BleDeviceInfo)

#endif // BLESCANTYPES_H
