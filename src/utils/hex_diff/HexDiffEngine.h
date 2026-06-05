/**
 * @file HexDiffEngine.h
 * @brief 十六进制差异引擎 — 字节级对比/三路比较/补丁生成
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 逐字节 diff、连续差异块分组、并排 hex dump、统一差异格式、
 * 上下文行扩展、三路比较、补丁生成与应用、颜色标记、统计。
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
 * @brief 十六进制差异引擎 — 字节级精确比较、三路合并、补丁操作
 *
 * compare() 逐字节对比返回 DiffEntry; compareBlocks() 合并连续差异块;
 * threeWayCompare() 三路合并; generatePatch()/applyPatch() 补丁操作;
 * sideBySideHexDump() 并排 hex dump; unifiedDiff() 统一差异格式。
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

    /** @brief 逐字节比较两个 QByteArray，发射 comparisonComplete/diffBlockFound 信号 */
    QList<DiffEntry> compare(const QByteArray &dataA, const QByteArray &dataB);

    /** @brief 将差异条目合并为连续差异块（相邻同类型合并） */
    QList<DiffBlock> compareBlocks(const QList<DiffEntry> &entries) const;

    /** @brief 并排 hex dump（类 hexdump -C 格式），差异用 '!' 插入 '+' 删除 '-' 标记 */
    QString sideBySideHexDump(const QByteArray &dataA, const QByteArray &dataB,
                              int bytesPerLine = 16) const;

    /** @brief 带上下文行的差异块文本，contextBytes 为上下文字节数 */
    QString contextDiff(const QByteArray &dataA, const QByteArray &dataB,
                        int contextBytes = 8) const;

    /** @brief 统一差异格式输出，labelA/labelB 为数据标签 */
    QString unifiedDiff(const QByteArray &dataA, const QByteArray &dataB,
                        const QString &labelA = "A",
                        const QString &labelB = "B") const;

    /** @brief 三路比较（base/A/B），标记共识与冲突，返回共识率 */
    ThreeWayResult threeWayCompare(const QByteArray &base,
                                   const QByteArray &dataA,
                                   const QByteArray &dataB);

    /** @brief 生成将 dataA 变换为 dataB 的补丁条目列表 */
    QList<PatchEntry> generatePatch(const QByteArray &dataA,
                                    const QByteArray &dataB);

    /** @brief 应用补丁到 data，verify=true 时校验原始字节，失败返回空 QByteArray */
    QByteArray applyPatch(const QByteArray &data,
                          const QList<PatchEntry> &patch,
                          bool verify = true);

    /** @brief 获取累计统计计数器快照 */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器为初始值 */
    void resetStatistics();

signals:
    /** @brief 比较完成时发射，similarity 为相似度百分比 (0.0~100.0) */
    void comparisonComplete(double similarity);

    /** @brief 发现差异块时逐个发射 */
    void diffBlockFound(quint64 offset, int length);

private:
    static QString formatHexByte(quint8 byte);    ///< 单字节→两位大写Hex
    static QString formatOffset(quint64 offset);  ///< 偏移→8位大写Hex

    Stats m_stats;                ///< 累计统计计数器
    double m_similaritySum = 0.0; ///< 累计相似度总和（用于计算平均值）
};

#endif // HEXDIFFENGINE_H
