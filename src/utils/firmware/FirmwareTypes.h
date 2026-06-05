/**
 * @file FirmwareTypes.h
 * @brief 固件差异分析公共数据类型
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 定义 FirmwareRegion / FirmwareDiffBlock / FirmwareDiffResult
 * 等固件二进制对比所需的共享结构体。
 */

#ifndef FIRMWARETYPES_H
#define FIRMWARETYPES_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QtGlobal>

/**
 * @struct FirmwareRegion
 * @brief 固件中的一个连续区域（段）
 *
 * 可表示 Flash 中的代码段、数据段、配置区等。
 */
struct FirmwareRegion {
    quint64    startAddress = 0;  ///< 区域起始地址
    quint64    size         = 0;  ///< 区域字节大小
    QString    name;              ///< 区域名称（如 ".text", ".data"）
    QByteArray data;              ///< 区域原始二进制数据
};

/**
 * @struct FirmwareDiffBlock
 * @brief 固件差异中的单个连续差异块
 *
 * 表示两份固件在某地址处连续 size 字节内容不同。
 * added/removed 两种边界情况由 size==0 + 空数据组合表示。
 */
struct FirmwareDiffBlock {
    quint64    address = 0;  ///< 差异块起始地址
    int        size    = 0;  ///< 差异块字节长度
    QByteArray dataA;        ///< 固件 A 中该块数据
    QByteArray dataB;        ///< 固件 B 中该块数据
};

/**
 * @struct FirmwareDiffResult
 * @brief 完整固件对比结果
 *
 * 包含统计数据（总块数、变化/未变化/新增/移除块数）、
 * 相似度百分比以及所有差异块列表。
 */
struct FirmwareDiffResult {
    int     totalBlocks    = 0;    ///< 对比总块数
    int     changedBlocks  = 0;    ///< 内容变化的块数
    int     unchangedBlocks = 0;   ///< 内容相同的块数
    int     addedBlocks    = 0;    ///< 仅存在于 B 的块数
    int     removedBlocks  = 0;    ///< 仅存在于 A 的块数
    double  similarity     = 0.0;  ///< 相似度 (0.0~100.0)

    QVector<FirmwareDiffBlock> diffs;  ///< 所有差异块列表
};

#endif // FIRMWARETYPES_H
