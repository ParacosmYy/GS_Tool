/**
 * @file FirmwareDiffer.h
 * @brief 固件二进制差异比较引擎
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 加载两份固件二进制文件，执行逐字节/对齐比较，
 * 生成差异块列表、相似度等统计信息。
 */

#ifndef FIRMWAREDIFFER_H
#define FIRMWAREDIFFER_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "utils/firmware/FirmwareTypes.h"

/**
 * @class FirmwareDiffer
 * @brief 固件二进制差异比较引擎
 *
 * 加载两份固件 BIN 文件，按可配置对齐粒度（1/2/4/8 字节）
 * 逐块比较，合并连续差异，计算相似度，并维护累计统计计数器。
 */
class FirmwareDiffer : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造固件差异引擎 @param parent 父对象 */
    explicit FirmwareDiffer(QObject *parent = nullptr);

    /** @brief 加载固件 A（BIN 文件） @param filePath 文件路径 @return 成功 true */
    bool loadFirmwareA(const QString &filePath);

    /** @brief 加载固件 B（BIN 文件） @param filePath 文件路径 @return 成功 true */
    bool loadFirmwareB(const QString &filePath);

    /** @brief 对已加载的两份固件执行全范围比较 @return 差异结果 */
    FirmwareDiffResult compare();

    /**
     * @brief 对指定地址范围执行区域比较
     * @param start 起始偏移（相对固件头部）
     * @param size  区域字节长度
     * @return 差异结果
     */
    FirmwareDiffResult compareRegion(quint64 start, quint64 size);

    /** @brief 设置对齐粒度（1/2/4/8 字节） @param bytes 对齐字节数 */
    void setAlignment(int bytes);

    /** @brief 获取当前对齐粒度 @return 对齐字节数 */
    int alignment() const;

    /**
     * @brief 设置比较时忽略的字节值列表
     * @param ignoreList 忽略的字节值（如 0xFF 填充区）
     */
    void setIgnoreBytes(const QVector<quint8> &ignoreList);

    /** @brief 获取固件 A 原始数据 @return 固件 A 的 QByteArray */
    QByteArray firmwareA() const;

    /** @brief 获取固件 B 原始数据 @return 固件 B 的 QByteArray */
    QByteArray firmwareB() const;

    /** @brief 获取固件 A 文件路径 @return 文件路径 */
    QString filePathA() const;

    /** @brief 获取固件 B 文件路径 @return 文件路径 */
    QString filePathB() const;

    /** @brief 固件 A 是否已加载 @return 已加载 true */
    bool isLoadedA() const;

    /** @brief 固件 B 是否已加载 @return 已加载 true */
    bool isLoadedB() const;

    // ---- 统计计数接口 ----

    /** @brief 累计 compare/compareRegion 调用次数 */
    quint64 totalComparisons() const;

    /** @brief 累计比较字节数 */
    quint64 totalBytesCompared() const;

    /** @brief 累计发现的差异块数 */
    quint64 totalDiffsFound() const;

    /** @brief 累计生成的补丁块数 */
    quint64 totalPatchesGenerated() const;

    /** @brief 重置所有累计统计计数器 */
    void resetStatistics();

signals:
    /** @brief 固件 A 加载完成 @param path 文件路径 */
    void firmwareALoaded(const QString &path);

    /** @brief 固件 B 加载完成 @param path 文件路径 */
    void firmwareBLoaded(const QString &path);

    /** @brief 比较完成 @param result 差异结果 */
    void comparisonComplete(const FirmwareDiffResult &result);

private:
    /**
     * @brief 核心比较实现
     * @param dataA 固件 A 数据
     * @param dataB 固件 B 数据
     * @param baseAddress 基地址偏移
     * @return 差异结果
     */
    FirmwareDiffResult doCompare(const QByteArray &dataA,
                                 const QByteArray &dataB,
                                 quint64 baseAddress) const;

    QByteArray m_firmwareA;              ///< 固件 A 原始数据
    QByteArray m_firmwareB;              ///< 固件 B 原始数据
    QString    m_filePathA;              ///< 固件 A 文件路径
    QString    m_filePathB;              ///< 固件 B 文件路径
    int        m_alignment = 1;          ///< 对齐粒度（字节）
    QVector<quint8> m_ignoreBytes;       ///< 忽略的字节值列表

    // ---- 统计计数器 ----
    quint64 m_totalComparisons    = 0;   ///< 累计比较调用次数
    quint64 m_totalBytesCompared  = 0;   ///< 累计比较字节数
    quint64 m_totalDiffsFound     = 0;   ///< 累计差异块数
    quint64 m_totalPatchesGenerated = 0; ///< 累计补丁块数
};

#endif // FIRMWAREDIFFER_H
