/**
 * @file HexDiffEngine.h
 * @brief 十六进制差异引擎 — 字节级对比/三路比较/补丁生成
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 对 QByteArray 执行字节级精确比较，支持:
 * - 逐字节 diff 结果生成（匹配/不匹配/插入/删除）
 * - 连续差异块分组
 * - 并排 hex dump 输出（类 hexdump -C 格式）
 * - 统一差异格式文本输出
 * - 上下文行扩展（差异块周围 N 字节）
 * - 三路比较（base/A/B 共识与冲突）
 * - 补丁生成与应用（将 dataA 变换为 dataB）
 * - 颜色标记（匹配/差异/插入/删除）
 * - 统计计数器与信号通知
 */

#ifndef HEXDIFFENGINE_H
#define HEXDIFFENGINE_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

/**
 * @class HexDiffEngine
 * @brief 十六进制差异引擎，提供字节级精确比较和补丁操作
 *
 * 核心功能:
 * - compare() 逐字节对比，返回 DiffEntry 列表
 * - compareBlocks() 将连续差异合并为 DiffBlock
 * - threeWayCompare() 三路合并比较
 * - generatePatch() / applyPatch() 补丁生成与应用
 * - sideBySideHexDump() 并排 hex dump 文本
 * - unifiedDiff() 统一差异格式输出
 * 统计计数器跟踪所有累计指标。
 */
class HexDiffEngine : public QObject
{
    Q_OBJECT

public:
    /** @brief 差异条目类型 */
    enum class DiffKind : quint8 {
        Match = 0,    ///< 字节完全匹配
        Mismatch,     ///< 同偏移字节不同
        Inserted,     ///< 仅存在于数据 B
        Deleted       ///< 仅存在于数据 A
    };
    Q_ENUM(DiffKind)

    /** @brief 颜色标记（用于文本输出） */
    enum class ColorCode : quint8 {
        Normal = 0,   ///< 匹配字节 — 无色
        Red,          ///< 差异字节
        Green,        ///< 插入字节
        Blue          ///< 删除字节
    };
    Q_ENUM(ColorCode)

    /** @brief 逐字节差异条目 */
    struct DiffEntry {
        quint64 offset = 0;          ///< 偏移地址
        quint8 byteA = 0;            ///< 数据 A 该位置的字节值
        quint8 byteB = 0;            ///< 数据 B 该位置的字节值
        DiffKind kind = DiffKind::Match;  ///< 差异类型
        ColorCode color = ColorCode::Normal;  ///< 颜色标记
    };

    /** @brief 连续差异块 */
    struct DiffBlock {
        quint64 startOffset = 0;     ///< 块起始偏移
        int length = 0;              ///< 块字节长度
        DiffKind kind = DiffKind::Mismatch;  ///< 块主类型
        QByteArray dataA;            ///< 数据 A 中该块字节
        QByteArray dataB;            ///< 数据 B 中该块字节
    };

    /** @brief 补丁操作类型 */
    enum class PatchOp : quint8 {
        Equal = 0,    ///< 保持不变
        Replace,      ///< 替换字节
        Insert,       ///< 插入字节
        Delete         ///< 删除字节
    };
    Q_ENUM(PatchOp)

    /** @brief 补丁条目 */
    struct PatchEntry {
        quint64 offset = 0;          ///< 应用偏移
        PatchOp op = PatchOp::Equal; ///< 操作类型
        QByteArray originalData;     ///< 原始字节（用于校验）
        QByteArray newData;          ///< 新字节
    };

    /** @brief 三路比较结果 */
    struct ThreeWayResult {
        QList<DiffEntry> entries;    ///< 差异条目列表
        int consensusBytes = 0;      ///< 三方一致的字节数
        int conflictBytes = 0;       ///< 三方冲突的字节数
        double consensusRate = 0.0;  ///< 共识率 (0~100)
    };

    /** @brief 累计统计计数器 */
    struct Stats {
        quint64 totalComparisons = 0;     ///< 累计比较调用次数
        quint64 totalBytesCompared = 0;   ///< 累计比较字节数
        quint64 totalMatches = 0;         ///< 累计匹配字节数
        quint64 totalMismatches = 0;      ///< 累计不匹配字节数
        quint64 totalInsertions = 0;      ///< 累计插入字节数
        quint64 totalDeletions = 0;       ///< 累计删除字节数
        double avgSimilarity = 0.0;       ///< 平均相似度 (0~100)
        quint64 totalPatchesGenerated = 0;  ///< 累计生成补丁数
        quint64 totalPatchesApplied = 0;    ///< 累计应用补丁数
    };

    /** @brief 构造十六进制差异引擎 @param parent 父对象 */
    explicit HexDiffEngine(QObject *parent = nullptr);

    /**
     * @brief 逐字节比较两个 QByteArray
     * @param dataA 数据缓冲区 A
     * @param dataB 数据缓冲区 B
     * @return 逐字节差异条目列表
     *
     * 完成后发射 comparisonComplete 信号，每个连续差异块
     * 发射 diffBlockFound 信号。
     */
    QList<DiffEntry> compare(const QByteArray &dataA, const QByteArray &dataB);

    /**
     * @brief 将差异条目合并为连续差异块
     * @param entries 逐字节差异条目列表
     * @return 连续差异块列表
     *
     * 将相邻的相同 DiffKind 条目合并为一个 DiffBlock。
     */
    QList<DiffBlock> compareBlocks(const QList<DiffEntry> &entries) const;

    /**
     * @brief 生成并排 hex dump 文本
     * @param dataA 数据 A
     * @param dataB 数据 B
     * @param bytesPerLine 每行字节数（默认 16）
     * @return 类 hexdump -C 格式的多行文本
     */
    QString sideBySideHexDump(const QByteArray &dataA, const QByteArray &dataB,
                              int bytesPerLine = 16) const;

    /**
     * @brief 生成带上下文行的差异块文本
     * @param dataA 数据 A
     * @param dataB 数据 B
     * @param contextBytes 上下文字节数（默认 8）
     * @return 仅包含差异块及其上下文的文本
     */
    QString contextDiff(const QByteArray &dataA, const QByteArray &dataB,
                        int contextBytes = 8) const;

    /**
     * @brief 生成统一差异格式输出
     * @param dataA 数据 A
     * @param dataB 数据 B
     * @param labelA 数据 A 标签
     * @param labelB 数据 B 标签
     * @return 统一差异格式文本
     */
    QString unifiedDiff(const QByteArray &dataA, const QByteArray &dataB,
                        const QString &labelA = "A",
                        const QString &labelB = "B") const;

    /**
     * @brief 执行三路比较
     * @param base 基准数据
     * @param dataA 分支 A
     * @param dataB 分支 B
     * @return 三路比较结果（共识/冲突统计）
     *
     * 三路比较以 base 为基准，比较 A 和 B 相对 base 的变化，
     * 标记三方一致（共识）或分歧（冲突）的字节。
     */
    ThreeWayResult threeWayCompare(const QByteArray &base,
                                   const QByteArray &dataA,
                                   const QByteArray &dataB);

    /**
     * @brief 生成将 dataA 变换为 dataB 的补丁
     * @param dataA 原始数据
     * @param dataB 目标数据
     * @return 补丁条目列表
     */
    QList<PatchEntry> generatePatch(const QByteArray &dataA,
                                    const QByteArray &dataB);

    /**
     * @brief 应用补丁到数据
     * @param data 原始数据
     * @param patch 补丁条目列表
     * @param verify 是否校验原始字节（默认 true）
     * @return 应用补丁后的数据；校验失败返回空 QByteArray
     */
    QByteArray applyPatch(const QByteArray &data,
                          const QList<PatchEntry> &patch,
                          bool verify = true);

    /**
     * @brief 获取累计统计计数器快照
     * @return 统计结构体副本
     */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器为初始值 */
    void resetStatistics();

signals:
    /**
     * @brief 比较完成时发射
     * @param similarity 相似度百分比 (0.0 ~ 100.0)
     */
    void comparisonComplete(double similarity);

    /**
     * @brief 发现差异块时逐个发射
     * @param offset 差异块起始偏移
     * @param length 差异块字节长度
     */
    void diffBlockFound(quint64 offset, int length);

private:
    /**
     * @brief 格式化单字节为两位大写十六进制
     * @param byte 字节值
     * @return 两位十六进制字符串
     */
    static QString formatHexByte(quint8 byte);

    /**
     * @brief 格式化偏移地址为8位十六进制
     * @param offset 偏移值
     * @return 8位十六进制字符串
     */
    static QString formatOffset(quint64 offset);

    Stats m_stats;                ///< 累计统计计数器
    double m_similaritySum = 0.0; ///< 累计相似度总和（用于计算平均值）
};

#endif // HEXDIFFENGINE_H
