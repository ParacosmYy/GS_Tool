/**
 * @file HoughTransform.cpp
 * @brief 霍夫变换直线检测实现
 */

#include "HoughTransform.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

HoughTransform::HoughTransform(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_thetaRes(M_PI / 180.0)
    , m_rhoRes(1.0)
{
}

QVector<HoughTransform::Line> HoughTransform::detect(
    const QVector<QVector<int>>& image, int threshold)
{
    QElapsedTimer timer;
    timer.start();

    if (image.isEmpty() || image[0].isEmpty()) {
        m_stats.totalDetected++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalDetected > 0) ? m_timeSum / m_stats.totalDetected : 0.0;
        emit detectionCompleted(0);
        return {};
    }

    int rows = image.size();
    int cols = image[0].size();

    /* 计算参数空间尺寸 */
    int thetaBins = static_cast<int>(M_PI / m_thetaRes);
    double diag = std::sqrt(static_cast<double>(rows * rows + cols * cols));
    double maxRho = diag;
    int rhoBins = static_cast<int>(2.0 * maxRho / m_rhoRes) + 1;

    /* 初始化累加器 */
    QVector<QVector<int>> accumulator(thetaBins, QVector<int>(rhoBins, 0));

    /* 对每个边缘点投票 */
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (image[r][c] != 0) {
                vote(r, c, accumulator, thetaBins, maxRho);
            }
        }
    }

    /* 提取峰值 */
    QVector<Line> lines = extractPeaks(accumulator, threshold);

    /* 按票数降序排序 */
    std::sort(lines.begin(), lines.end(),
              [](const Line& a, const Line& b) {
                  return a.votes > b.votes;
              });

    /* 统计更新 */
    m_stats.totalDetected++;
    m_stats.totalLines += lines.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalDetected > 0) ? m_timeSum / m_stats.totalDetected : 0.0;

    emit detectionCompleted(lines.size());
    return lines;
}

void HoughTransform::setResolution(double thetaRes, double rhoRes)
{
    if (thetaRes > 0.0) m_thetaRes = thetaRes;
    if (rhoRes > 0.0) m_rhoRes = rhoRes;
}

void HoughTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

void HoughTransform::vote(int row, int col,
                           QVector<QVector<int>>& accumulator,
                           int thetaBins, double maxRho) const
{
    for (int t = 0; t < thetaBins; ++t) {
        double theta = t * m_thetaRes;
        double rho = col * std::cos(theta) + row * std::sin(theta);

        /* 将rho映射到累加器索引 */
        int rhoIdx = static_cast<int>((rho + maxRho) / m_rhoRes);
        if (rhoIdx >= 0 && rhoIdx < accumulator[t].size()) {
            accumulator[t][rhoIdx]++;
        }
    }
}

QVector<HoughTransform::Line> HoughTransform::extractPeaks(
    const QVector<QVector<int>>& accumulator, int threshold) const
{
    QVector<Line> lines;

    int thetaBins = accumulator.size();
    for (int t = 0; t < thetaBins; ++t) {
        int rhoBins = accumulator[t].size();
        for (int r = 0; r < rhoBins; ++r) {
            int votes = accumulator[t][r];
            if (votes >= threshold) {
                /* 检查是否为局部最大值(3x3邻域) */
                bool isMax = true;
                for (int dt = -1; dt <= 1 && isMax; ++dt) {
                    for (int dr = -1; dr <= 1 && isMax; ++dr) {
                        if (dt == 0 && dr == 0) continue;
                        int nt = t + dt;
                        int nr = r + dr;
                        if (nt >= 0 && nt < thetaBins &&
                            nr >= 0 && nr < rhoBins) {
                            if (accumulator[nt][nr] > votes)
                                isMax = false;
                        }
                    }
                }

                if (isMax) {
                    Line line;
                    line.theta = t * m_thetaRes;
                    /* 还原rho值 */
                    int rhoBins = accumulator[t].size();
                    double maxRho = rhoBins * m_rhoRes / 2.0;
                    line.rho = r * m_rhoRes - maxRho;
                    line.votes = votes;
                    lines.append(line);
                }
            }
        }
    }

    return lines;
}
