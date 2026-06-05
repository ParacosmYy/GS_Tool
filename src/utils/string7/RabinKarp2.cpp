/**
 * @file RabinKarp2.cpp
 * @brief Rabin-Karp二维模式匹配实现 — 滚动哈希 + 行列分离
 */

#include "utils/string7/RabinKarp2.h"

#include <QElapsedTimer>

#include <algorithm>

/** @brief 构造函数 @param baseRow 行基数 @param baseCol 列基数 @param modulus 模数 @param parent 父对象 */
RabinKarp2::RabinKarp2(quint64 baseRow, quint64 baseCol, quint64 modulus, QObject* parent)
    : QObject(parent)
    , m_baseRow(baseRow)
    , m_baseCol(baseCol)
    , m_modulus(modulus)
{
}

/** @brief 模乘法(防止溢出) @param a 乘数 @param b 乘数 @return a*b mod m */
quint64 RabinKarp2::mulmod(quint64 a, quint64 b) const
{
    /* 使用__int128避免溢出 */
    return (static_cast<__uint128_t>(a) * b) % m_modulus;
}

/** @brief 预计算行哈希 @param text 文本矩阵 @param patCols 模式列数 @return 行哈希矩阵 */
std::vector<std::vector<quint64>> RabinKarp2::precomputeRowHashes(
    const std::vector<std::vector<int>>& text, int patCols) const
{
    int rows = static_cast<int>(text.size());
    int cols = (rows > 0) ? static_cast<int>(text[0].size()) : 0;
    if (patCols > cols) return {};

    /* 预计算 baseCol^patCols mod modulus */
    quint64 powCol = 1;
    for (int i = 0; i < patCols; ++i) powCol = mulmod(powCol, m_baseCol);

    std::vector<std::vector<quint64>> rowHashes(rows,
        std::vector<quint64>(cols - patCols + 1, 0));

    for (int r = 0; r < rows; ++r) {
        /* 第一个窗口的哈希 */
        quint64 h = 0;
        for (int c = 0; c < patCols; ++c) {
            h = (mulmod(h, m_baseCol) + static_cast<quint64>(text[r][c])) % m_modulus;
        }
        rowHashes[r][0] = h;

        /* 滚动计算后续窗口 */
        for (int c = 1; c <= cols - patCols; ++c) {
            quint64 outVal = mulmod(static_cast<quint64>(text[r][c - 1]), powCol);
            h = (mulmod(h, m_baseCol) + static_cast<quint64>(text[r][c + patCols - 1])
                 + m_modulus - outVal) % m_modulus;
            rowHashes[r][c] = h;
        }
    }

    return rowHashes;
}

/** @brief 预计算列哈希 @param rowHashes 行哈希矩阵 @param patRows 模式行数 @return 二维哈希矩阵 */
std::vector<std::vector<quint64>> RabinKarp2::precomputeColHashes(
    const std::vector<std::vector<quint64>>& rowHashes, int patRows) const
{
    int rows = static_cast<int>(rowHashes.size());
    int cols = (rows > 0) ? static_cast<int>(rowHashes[0].size()) : 0;
    if (patRows > rows) return {};

    /* 预计算 baseRow^patRows mod modulus */
    quint64 powRow = 1;
    for (int i = 0; i < patRows; ++i) powRow = mulmod(powRow, m_baseRow);

    std::vector<std::vector<quint64>> hashGrid(rows - patRows + 1,
        std::vector<quint64>(cols, 0));

    for (int c = 0; c < cols; ++c) {
        /* 第一个窗口的列哈希 */
        quint64 h = 0;
        for (int r = 0; r < patRows; ++r) {
            h = (mulmod(h, m_baseRow) + rowHashes[r][c]) % m_modulus;
        }
        hashGrid[0][c] = h;

        /* 滚动计算后续窗口 */
        for (int r = 1; r <= rows - patRows; ++r) {
            quint64 outVal = mulmod(rowHashes[r - 1][c], powRow);
            h = (mulmod(h, m_baseRow) + rowHashes[r + patRows - 1][c]
                 + m_modulus - outVal) % m_modulus;
            hashGrid[r][c] = h;
        }
    }

    return hashGrid;
}

/** @brief 计算矩阵哈希 @param matrix 输入矩阵 @return 哈希值 */
quint64 RabinKarp2::computeHash(const QVector<QVector<int>>& matrix) const
{
    if (matrix.isEmpty() || matrix[0].isEmpty()) return 0;

    int rows = matrix.size();
    int cols = matrix[0].size();

    /* 先计算每行的哈希 */
    std::vector<quint64> rowHashes(rows, 0);
    for (int r = 0; r < rows; ++r) {
        quint64 h = 0;
        for (int c = 0; c < cols; ++c) {
            h = (mulmod(h, m_baseCol) + static_cast<quint64>(matrix[r][c])) % m_modulus;
        }
        rowHashes[r] = h;
    }

    /* 再对行哈希做列哈希 */
    quint64 totalHash = 0;
    for (int r = 0; r < rows; ++r) {
        totalHash = (mulmod(totalHash, m_baseRow) + rowHashes[r]) % m_modulus;
    }

    return totalHash;
}

/** @brief 验证匹配 @param text 文本 @param pattern 模式 @param startRow 起始行 @param startCol 起始列 @return 是否匹配 */
bool RabinKarp2::verifyMatch(const QVector<QVector<int>>& text,
                              const QVector<QVector<int>>& pattern,
                              int startRow, int startCol) const
{
    int patRows = pattern.size();
    int patCols = (patRows > 0) ? pattern[0].size() : 0;

    for (int r = 0; r < patRows; ++r) {
        for (int c = 0; c < patCols; ++c) {
            if (text[startRow + r][startCol + c] != pattern[r][c]) return false;
        }
    }
    return true;
}

/** @brief 在QVector矩阵中搜索 @param text 文本 @param pattern 模式 @param verify 是否验证 @return 匹配列表 */
QVector<RabinKarp2::Match> RabinKarp2::search(const QVector<QVector<int>>& text,
                                                const QVector<QVector<int>>& pattern,
                                                bool verify)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Match> matches;

    if (text.isEmpty() || pattern.isEmpty()) return matches;
    if (pattern[0].isEmpty()) return matches;

    int textRows = text.size();
    int textCols = text[0].size();
    int patRows = pattern.size();
    int patCols = pattern[0].size();

    if (patRows > textRows || patCols > textCols) return matches;

    /* 转换为STL格式进行处理 */
    std::vector<std::vector<int>> textStd(textRows, std::vector<int>(textCols));
    std::vector<std::vector<int>> patStd(patRows, std::vector<int>(patCols));
    for (int r = 0; r < textRows; ++r)
        for (int c = 0; c < textCols; ++c) textStd[r][c] = text[r][c];
    for (int r = 0; r < patRows; ++r)
        for (int c = 0; c < patCols; ++c) patStd[r][c] = pattern[r][c];

    auto resultStd = search(textStd, patStd, verify);
    for (const auto& m : resultStd) {
        matches.append({m.row, m.col, m.similarity});
    }

    double elapsed = timer.elapsed();
    emit searchCompleted(matches.size(), elapsed);
    return matches;
}

/** @brief 在STL矩阵中搜索 @param text 文本 @param pattern 模式 @param verify 是否验证 @return 匹配列表 */
std::vector<RabinKarp2::Match> RabinKarp2::search(
    const std::vector<std::vector<int>>& text,
    const std::vector<std::vector<int>>& pattern,
    bool verify)
{
    QElapsedTimer timer;
    timer.start();

    std::vector<Match> matches;

    if (text.empty() || pattern.empty()) return matches;

    int textRows = static_cast<int>(text.size());
    int textCols = static_cast<int>(text[0].size());
    int patRows = static_cast<int>(pattern.size());
    int patCols = static_cast<int>(pattern[0].size());

    if (patRows > textRows || patCols > textCols) return matches;

    /* Step 1: 计算模式的哈希 */
    std::vector<std::vector<quint64>> patRowHashes(patRows,
        std::vector<quint64>(1, 0));
    for (int r = 0; r < patRows; ++r) {
        quint64 h = 0;
        for (int c = 0; c < patCols; ++c) {
            h = (mulmod(h, m_baseCol) + static_cast<quint64>(pattern[r][c])) % m_modulus;
        }
        patRowHashes[r][0] = h;
    }

    quint64 patternHash = 0;
    for (int r = 0; r < patRows; ++r) {
        patternHash = (mulmod(patternHash, m_baseRow) + patRowHashes[r][0]) % m_modulus;
    }

    /* Step 2: 预计算文本的行哈希 */
    auto rowHashes = precomputeRowHashes(text, patCols);
    if (rowHashes.empty()) return matches;

    /* Step 3: 预计算二维哈希 */
    auto hashGrid = precomputeColHashes(rowHashes, patRows);
    if (hashGrid.empty()) return matches;

    /* Step 4: 比较哈希并验证匹配 */
    int cellsScanned = 0;
    int hashCollisions = 0;

    for (int r = 0; r < static_cast<int>(hashGrid.size()); ++r) {
        for (int c = 0; c < static_cast<int>(hashGrid[0].size()); ++c) {
            ++cellsScanned;
            if (hashGrid[r][c] == patternHash) {
                bool isMatch = true;
                if (verify) {
                    /* 逐元素验证 */
                    isMatch = true;
                    for (int pr = 0; pr < patRows && isMatch; ++pr) {
                        for (int pc = 0; pc < patCols && isMatch; ++pc) {
                            if (text[r + pr][c + pc] != pattern[pr][pc]) {
                                isMatch = false;
                                ++hashCollisions;
                            }
                        }
                    }
                }

                if (isMatch) {
                    matches.push_back({r, c, 1.0});
                }
            }
        }
    }

    /* 更新统计 */
    m_stats.totalSearches++;
    m_stats.totalMatches += static_cast<int>(matches.size());
    m_stats.totalHashCollisions += hashCollisions;
    m_stats.totalCellsScanned += cellsScanned;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    return matches;
}

/** @brief 设置哈希参数 @param baseRow 行基数 @param baseCol 列基数 @param modulus 模数 */
void RabinKarp2::setHashParams(quint64 baseRow, quint64 baseCol, quint64 modulus)
{
    m_baseRow = std::max<quint64>(2, baseRow);
    m_baseCol = std::max<quint64>(2, baseCol);
    m_modulus = std::max<quint64>(100, modulus);
}

/** @brief 重置统计 */
void RabinKarp2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
