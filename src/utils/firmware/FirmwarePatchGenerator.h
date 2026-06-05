/**
 * @file FirmwarePatchGenerator.h
 * @brief 固件补丁生成器 — Intel HEX / BIN 补丁格式
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 从 FirmwareDiffResult 生成 Intel HEX 或纯二进制补丁，
 * 并提供补丁应用与 CRC 校验功能。
 */

#ifndef FIRMWAREPATCHGENERATOR_H
#define FIRMWAREPATCHGENERATOR_H

#include <QByteArray>
#include <QObject>
#include <QtGlobal>

#include "utils/firmware/FirmwareTypes.h"

/**
 * @class FirmwarePatchGenerator
 * @brief 固件补丁生成与应用工具
 *
 * generateIhexPatch()  生成 Intel HEX 格式补丁；
 * generateBinPatch()   生成纯二进制补丁（地址+长度+数据）；
 * applyPatch()         将补丁应用到原始固件数据；
 * 所有操作均含 CRC32 完整性校验。
 */
class FirmwarePatchGenerator : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造补丁生成器 @param parent 父对象 */
    explicit FirmwarePatchGenerator(QObject *parent = nullptr);

    /**
     * @brief 从差异结果生成 Intel HEX 格式补丁
     * @param diff 差异结果（仅使用 dataB 作为补丁数据）
     * @return Intel HEX 格式的文本数据（每行以 ':' 开头）
     */
    QByteArray generateIhexPatch(const FirmwareDiffResult &diff);

    /**
     * @brief 从差异结果生成纯二进制补丁
     *
     * 格式: [address:u64][size:u32][dataB:N] 重复，末尾 4 字节 CRC32
     * @param diff 差异结果
     * @return 二进制补丁数据
     */
    QByteArray generateBinPatch(const FirmwareDiffResult &diff);

    /**
     * @brief 将二进制补丁应用到原始固件
     * @param original 原始固件数据
     * @param patch generateBinPatch() 生成的补丁
     * @param result [out] 应用补丁后的固件数据
     * @return 成功 true（CRC 校验失败返回 false）
     */
    bool applyPatch(const QByteArray &original,
                    const QByteArray &patch,
                    QByteArray &result);

    // ---- 统计计数接口 ----

    /** @brief 累计生成补丁次数 */
    quint64 totalPatchesGenerated() const;

    /** @brief 累计应用补丁次数 */
    quint64 totalPatchesApplied() const;

    /** @brief 累计补丁总字节数 */
    quint64 totalPatchBytes() const;

    /** @brief 累计应用错误次数（CRC 失败等） */
    quint64 totalApplyErrors() const;

    /** @brief 重置所有累计统计计数器 */
    void resetStatistics();

private:
    /**
     * @brief 计算数据的 CRC32 校验和
     * @param data 输入数据
     * @return CRC32 值
     */
    static quint32 computeCrc32(const QByteArray &data);

    /**
     * @brief 生成一条 Intel HEX 记录行
     * @param byteCount 数据字节数
     * @param address 16 位地址
     * @param recordType 记录类型 (00=数据, 01=EOF, 04=扩展线性地址)
     * @param data 数据字节
     * @return 一行 HEX 记录（含 ':' 前缀和换行）
     */
    static QByteArray makeIhexLine(quint8 byteCount,
                                  quint16 address,
                                  quint8 recordType,
                                  const QByteArray &data);

    quint64 m_totalPatchesGenerated = 0;  ///< 累计生成补丁次数
    quint64 m_totalPatchesApplied   = 0;  ///< 累计应用补丁次数
    quint64 m_totalPatchBytes       = 0;  ///< 累计补丁总字节数
    quint64 m_totalApplyErrors      = 0;  ///< 累计应用错误次数
};

#endif // FIRMWAREPATCHGENERATOR_H
