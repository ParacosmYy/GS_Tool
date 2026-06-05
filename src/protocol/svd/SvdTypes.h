/**
 * @file SvdTypes.h
 * @brief CMSIS SVD(System View Description)寄存器描述数据结构
 *
 * 定义 SVD 文件的层级数据模型: SvdField → SvdRegister → SvdCluster
 * → SvdPeripheral → SvdDevice。纯 C++ 结构体，无 Q_OBJECT。
 * 供 SvdParser 解析输出、SvdRegisterModel 显示使用。
 */
#ifndef SVD_TYPES_H
#define SVD_TYPES_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QtGlobal>

/**
 * @brief SVD 字段枚举值定义
 *
 * 描述寄存器字段的可选命名值，例如枚举 {0="Disable", 1="Enable"}。
 */
struct SvdEnumeratedValue {
    QString name;                ///< 枚举项名称
    QString description;         ///< 枚举项描述
    quint64 value = 0;           ///< 枚举原始值
};

/**
 * @brief SVD 寄存器字段
 *
 * 描述寄存器中一个位域，包含位置、宽度和可选的枚举值映射。
 */
struct SvdField {
    QString name;                ///< 字段名称 (如 "MODER0")
    QString description;         ///< 字段描述
    quint32 bitOffset = 0;       ///< 位偏移量 (最低位位置)
    quint32 bitWidth  = 0;       ///< 位宽度 (占几位)
    QString access;              ///< 访问权限: "read-write"/"read-only"/"write-only"
    QVector<SvdEnumeratedValue> enumeratedValues; ///< 枚举值列表
};

/**
 * @brief SVD 寄存器
 *
 * 描述一个外设寄存器，包含地址偏移、大小和位域字段列表。
 */
struct SvdRegister {
    QString name;                ///< 寄存器名称 (如 "MODER")
    QString displayName;         ///< 显示名称 (可选，更友好)
    QString description;         ///< 寄存器描述
    quint32 addressOffset = 0;   ///< 相对外设基地址的偏移
    quint32 size         = 32;   ///< 寄存器位宽 (通常 8/16/32/64)
    QString access;              ///< 访问权限
    quint64 resetValue   = 0;    ///< 复位值
    quint64 resetMask    = 0;    ///< 复位掩码 (0 表示未指定)
    QVector<SvdField> fields;    ///< 位域字段列表
};

/**
 * @brief SVD 寄存器簇
 *
 * 将一组寄存器按固定偏移量分组，支持多簇实例。
 */
struct SvdCluster {
    QString name;                ///< 簇名称
    QString description;         ///< 簇描述
    quint32 addressOffset = 0;   ///< 簇地址偏移
    quint32 size         = 0;    ///< 簇内寄存器默认大小
    QVector<SvdRegister> registers;       ///< 簇内寄存器列表
    QVector<SvdCluster>  clusters;        ///< 嵌套子簇 (递归)
};

/**
 * @brief SVD 外设
 *
 * 描述一个 MCU 外设 (如 GPIOA、USART1)，包含基地址和寄存器/簇列表。
 */
struct SvdPeripheral {
    QString name;                ///< 外设名称 (如 "GPIOA")
    QString displayName;         ///< 显示名称
    QString description;         ///< 外设描述
    quint32 baseAddress  = 0;    ///< 基地址 (绝对地址)
    quint32 size         = 32;   ///< 默认寄存器位宽
    QString access;              ///< 默认访问权限
    QString groupName;           ///< 外设分组名 (如 "GPIO")
    QVector<SvdRegister> registers;       ///< 寄存器列表
    QVector<SvdCluster>  clusters;        ///< 簇列表
};

/**
 * @brief SVD 设备
 *
 * SVD 文件根节点，描述整个 MCU 的外设和寄存器拓扑。
 */
struct SvdDevice {
    QString vendor;              ///< 厂商名称 (如 "STMicroelectronics")
    QString name;                ///< 设备名称 (如 "STM32F407")
    QString description;         ///< 设备描述
    quint32 addressUnitBits = 8; ///< 最小寻址单位位数 (通常 8)
    quint32 width           = 32; ///< 默认寄存器位宽
    quint64 size            = 32; ///< 默认大小
    QString access;              ///< 默认访问权限
    quint64 resetValue      = 0; ///< 默认复位值
    quint64 resetMask       = 0; ///< 默认复位掩码
    QVector<SvdPeripheral> peripherals; ///< 外设列表
};

/**
 * @brief SVD 解析统计信息
 *
 * 跟踪解析器累计处理的外设/寄存器/字段数量和错误计数。
 */
struct SvdParseStatistics {
    quint64 totalPeripherals = 0; ///< 累计解析外设总数
    quint64 totalRegisters   = 0; ///< 累计解析寄存器总数
    quint64 totalFields      = 0; ///< 累计解析字段总数
    quint64 totalClusters    = 0; ///< 累计解析簇总数
    quint64 totalParseErrors = 0; ///< 累计解析错误总数
    quint64 totalFilesLoaded = 0; ///< 累计加载文件总数
};

#endif // SVD_TYPES_H
