/**
 * @file RegisterMapTypes.h
 * @brief 寄存器地图数据类型定义
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 定义寄存器位域、寄存器条目和寄存器地图的公共数据结构，
 * 被 RegisterMapModel 和 RegisterMapEditor 共用。
 */

#ifndef REGISTERMAPTYPES_H
#define REGISTERMAPTYPES_H

#include <QString>
#include <QVector>
#include <QtGlobal>

/**
 * @struct RegisterField
 * @brief 寄存器内单个位域的定义
 */
struct RegisterField {
    QString name;           ///< 位域名称
    int bitPos = 0;         ///< 起始位位置（低位）
    int bitWidth = 1;       ///< 位宽（位数）
    quint64 resetValue = 0; ///< 复位默认值
    QString description;    ///< 位域描述/说明
};

/**
 * @struct RegisterEntry
 * @brief 单个寄存器的完整描述
 */
struct RegisterEntry {
    quint32 address = 0;                ///< 寄存器地址
    QString name;                       ///< 寄存器名称
    int width = 32;                     ///< 寄存器位宽 (8/16/32/64)
    quint64 resetValue = 0;             ///< 复位值
    QVector<RegisterField> fields;      ///< 位域列表
    QString groupName;                  ///< 所属分组名称
};

/**
 * @struct RegisterMap
 * @brief 设备完整的寄存器地图
 */
struct RegisterMap {
    QString deviceName;                 ///< 设备/芯片名称
    int addressWidth = 32;              ///< 地址位宽
    int dataWidth = 32;                 ///< 数据位宽
    QVector<RegisterEntry> registers;   ///< 寄存器列表
};

#endif // REGISTERMAPTYPES_H
