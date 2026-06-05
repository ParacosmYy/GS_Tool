/**
 * @file RidgeDetector.cpp
 * @brief 脊线检测引擎实现 — 2D/1D脊线追踪与连续性约束
 */

#include "utils/peakdetect/RidgeDetector.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RidgeDetector::RidgeDetector(QObject* parent)
    : QObject(parent)
    , m_amplitudeThreshold(0.1)
    , m_minRidgeLength(3)
    , m_maxColumnJump(2)
    , m_lengthSum(0.0)
{
}

/** @brief 设置幅度阈值 @param threshold 阈值 */
void RidgeDetector::setAmplitudeThreshold(double threshold)
{
    m_amplitudeThreshold = threshold;
}

/** @brief 设置最小连续长度 @param length 最小行数 */
void RidgeDetector::setMinRidgeLength(int length)
{
    m_minRidgeLength = qMax(1, length);
}

/** @brief 设置最大列跳变 @param jump 最大跳变 */
void RidgeDetector::setMaxColumnJump(int jump)
{
    m_maxColumnJump = qMax(1, jump);
}

/** @brief 检测2D脊线 @param data 2D数据 @param cols 列数 @return 脊线列表 */
QList<RidgeDetector::Ridge> RidgeDetector::detect(
    const QVector<double>& data, int cols)
{
    QList<Ridge> ridges;
    if (data.isEmpty() || cols < 1) return ridges;

    int rows = data.size() / cols;
    if (rows < 1) return ridges;

    /* 每行找峰值列 */
    QVector<int> peakCols(rows, -1);
    for (int r = 0; r < rows; ++r) {
        double maxVal = -1e300;
        int maxCol = 0;
        for (int c = 0; c < cols; ++c) {
            double val = data[r * cols + c];
            if (val > maxVal) {
                maxVal = val;
                maxCol = c;
            }
        }
        if (maxVal >= m_amplitudeThreshold) {
            peakCols[r] = maxCol;
        }
    }

    /* 追踪连续脊线 */
    int r = 0;
    while (r < rows) {
        if (peakCols[r] < 0) { ++r; continue; }

        Ridge ridge;
        ridge.startRow = r;
        ridge.startCol = peakCols[r];
        ridge.peakAmplitude = data[r * cols + peakCols[r]];
        double ampSum = ridge.peakAmplitude;
        int length = 1;

        int prevCol = peakCols[r];
        int nextRow = r + 1;

        while (nextRow < rows && peakCols[nextRow] >= 0
               && qAbs(peakCols[nextRow] - prevCol) <= m_maxColumnJump) {
            double val = data[nextRow * cols + peakCols[nextRow]];
            if (val > ridge.peakAmplitude) {
                ridge.peakAmplitude = val;
            }
            ampSum += val;
            prevCol = peakCols[nextRow];
            ++length;
            ++nextRow;
        }

        if (length >= m_minRidgeLength) {
            ridge.endRow = nextRow - 1;
            ridge.endCol = prevCol;
            ridge.length = length;
            ridge.averageAmplitude = ampSum / length;
            ridges.append(ridge);
        }

        r = nextRow;
    }

    /* 更新统计 */
    ++m_stats.totalScans;
    m_stats.totalRidgesFound += static_cast<quint64>(ridges.size());
    for (const auto& ridge : ridges) {
        m_lengthSum += ridge.length;
        if (ridge.length > m_stats.longestRidge) {
            m_stats.longestRidge = ridge.length;
        }
        if (ridge.peakAmplitude > m_stats.peakAmplitude) {
            m_stats.peakAmplitude = ridge.peakAmplitude;
        }
    }
    if (m_stats.totalRidgesFound > 0) {
        m_stats.averageRidgeLength = m_lengthSum
            / static_cast<double>(m_stats.totalRidgesFound);
    }

    emit ridgesDetected(ridges.size());
    return ridges;
}

/** @brief 检测1D序列脊线 @param sequence 序列 @return 脊线列表 */
QList<RidgeDetector::Ridge> RidgeDetector::detectSequence(
    const QVector<double>& sequence)
{
    QList<Ridge> ridges;
    if (sequence.size() < m_minRidgeLength) return ridges;

    int n = sequence.size();
    int start = -1;
    double ampSum = 0.0;
    double peakAmp = -1e300;

    for (int i = 0; i < n; ++i) {
        bool above = sequence[i] >= m_amplitudeThreshold;

        if (above && start < 0) {
            /* 开始新脊线 */
            start = i;
            ampSum = sequence[i];
            peakAmp = sequence[i];
        } else if (above && start >= 0) {
            /* 延续脊线 */
            ampSum += sequence[i];
            if (sequence[i] > peakAmp) peakAmp = sequence[i];
        } else if (!above && start >= 0) {
            /* 结束脊线 */
            int length = i - start;
            if (length >= m_minRidgeLength) {
                Ridge ridge;
                ridge.startRow = start;
                ridge.endRow = i - 1;
                ridge.startCol = 0;
                ridge.endCol = 0;
                ridge.length = length;
                ridge.peakAmplitude = peakAmp;
                ridge.averageAmplitude = ampSum / length;
                ridges.append(ridge);
            }
            start = -1;
        }
    }

    /* 处理最后一段 */
    if (start >= 0) {
        int length = n - start;
        if (length >= m_minRidgeLength) {
            Ridge ridge;
            ridge.startRow = start;
            ridge.endRow = n - 1;
            ridge.startCol = 0;
            ridge.endCol = 0;
            ridge.length = length;
            ridge.peakAmplitude = peakAmp;
            ridge.averageAmplitude = ampSum / length;
            ridges.append(ridge);
        }
    }

    ++m_stats.totalScans;
    m_stats.totalRidgesFound += static_cast<quint64>(ridges.size());
    for (const auto& ridge : ridges) {
        m_lengthSum += ridge.length;
        if (ridge.length > m_stats.longestRidge) {
            m_stats.longestRidge = ridge.length;
        }
        if (ridge.peakAmplitude > m_stats.peakAmplitude) {
            m_stats.peakAmplitude = ridge.peakAmplitude;
        }
    }
    if (m_stats.totalRidgesFound > 0) {
        m_stats.averageRidgeLength = m_lengthSum
            / static_cast<double>(m_stats.totalRidgesFound);
    }

    emit ridgesDetected(ridges.size());
    return ridges;
}

/** @brief 重置统计 */
void RidgeDetector::resetStatistics()
{
    m_stats = Stats{};
    m_lengthSum = 0.0;
}
