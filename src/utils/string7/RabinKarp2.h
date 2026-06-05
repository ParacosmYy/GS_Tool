/**
 * @file RabinKarp2.h
 * @brief Rabin-Karp二维模式匹配 — 矩阵中的子矩阵搜索
 *
 * 功能: 实现Rabin-Karp二维模式匹配算法，在二维矩阵中
 *       高效搜索子矩阵模式。使用滚动哈希实现O(n*m*r)
 *       时间复杂度的二维模式匹配。支持多种哈希基数和
 *       模数选择，适用于模板匹配和数据校验场景。
 *
 * 协作: AhoCorasick(一维多模式匹配) / CrcStreamVerifier(校验)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <utility>
#include <vector>

/**
 * @brief Rabin-Karp二维模式匹配器
 */
class RabinKarp2 : public QObject {
    Q_OBJECT

public:
    /** @brief 匹配位置 */
    struct Match {
        int row;            ///< 匹配起始行
        int col;            ///< 匹配起始列
        double similarity;  ///< 相似度(1.0=完全匹配)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;           ///< 累计搜索次数
        int totalMatches = 0;            ///< 累计匹配数
        int totalHashCollisions = 0;     ///< 累计哈希碰撞次数
        int totalCellsScanned = 0;       ///< 累计扫描单元格数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param baseRow 行哈希基数(默认257)
     * @param baseCol 列哈希基数(默认263)
     * @param modulus 哈希模数(默认10^9+7)
     * @param parent 父对象
     */
    explicit RabinKarp2(quint64 baseRow = 257, quint64 baseCol = 263,
                        quint64 modulus = 1000000007, QObject* parent = nullptr);

    /**
     * @brief 在矩阵中搜索子矩阵模式
     * @param text 二维文本矩阵
     * @param pattern 搜索模式(子矩阵)
     * @param verify 是否逐元素验证哈希碰撞
     * @return 匹配位置列表
     */
    QVector<Match> search(const QVector<QVector<int>>& text,
                          const QVector<QVector<int>>& pattern,
                          bool verify = true);

    /**
     * @brief 在STL矩阵中搜索
     * @param text 二维文本矩阵
     * @param pattern 搜索模式
     * @param verify 是否验证
     * @return 匹配位置列表
     */
    std::vector<Match> search(const std::vector<std::vector<int>>& text,
                              const std::vector<std::vector<int>>& pattern,
                              bool verify = true);

    /**
     * @brief 计算矩阵的哈希值
     * @param matrix 输入矩阵
     * @return 哈希值
     */
    quint64 computeHash(const QVector<QVector<int>>& matrix) const;

    /**
     * @brief 检查两个子矩阵是否完全匹配
     * @param text 文本矩阵
     * @param pattern 模式矩阵
     * @param startRow 文本起始行
     * @param startCol 文本起始列
     * @return true=完全匹配
     */
    bool verifyMatch(const QVector<QVector<int>>& text,
                     const QVector<QVector<int>>& pattern,
                     int startRow, int startCol) const;

    /**
     * @brief 设置哈希参数
     * @param baseRow 行基数
     * @param baseCol 列基数
     * @param modulus 模数
     */
    void setHashParams(quint64 baseRow, quint64 baseCol, quint64 modulus);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param matchCount 匹配数 @param elapsedMs 耗时 */
    void searchCompleted(int matchCount, double elapsedMs);

private:
    /**
     * @brief 预计算行哈希(对每行用滚动哈希)
     * @param text 文本矩阵
     * @param patCols 模式列数
     * @return 行哈希矩阵
     */
    std::vector<std::vector<quint64>> precomputeRowHashes(
        const std::vector<std::vector<int>>& text, int patCols) const;

    /**
     * @brief 预计算列哈希(对行哈希矩阵再做列哈希)
     * @param rowHashes 行哈希矩阵
     * @param patRows 模式行数
     * @return 二维哈希矩阵
     */
    std::vector<std::vector<quint64>> precomputeColHashes(
        const std::vector<std::vector<quint64>>& rowHashes, int patRows) const;

    /** @brief 模乘法(防止溢出) @param a 乘数 @param b 乘数 @return a*b mod m */
    quint64 mulmod(quint64 a, quint64 b) const;

    quint64 m_baseRow;              ///< 行哈希基数
    quint64 m_baseCol;              ///< 列哈希基数
    quint64 m_modulus;              ///< 哈希模数

    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
