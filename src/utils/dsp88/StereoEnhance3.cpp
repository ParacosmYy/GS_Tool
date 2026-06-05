#include "StereoEnhance3.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class StereoEnhance3
 * @brief 立体声增强处理器实现
 *
 * 通过Mid-Side(M/S)处理扩展立体声声场宽度:
 * Mid = (L + R) / 2   (中央信息)
 * Side = (L - R) / 2  (侧向信息)
 *
 * 增强原理: 增强Side分量相对于Mid分量的比例。
 * width=0.5为原始立体声，width=1.0为最大扩展，
 * width=0.0为单声道。
 *
 * 输出: L' = Mid - Side * width, R' = Mid + Side * width
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
StereoEnhance3::StereoEnhance3(QObject* parent)
    : QObject(parent)
    , m_width(0.7)
{
}

/**
 * @brief 处理立体声帧
 *
 * 对左右声道进行M/S变换、Side增益调整、逆M/S变换:
 * 1. 计算Mid和Side信号
 * 2. Side乘以宽度系数(2*width)
 * 3. 防止混音后产生反相问题
 * 4. 重建左右声道输出
 *
 * @param left 左声道采样缓冲区
 * @param right 右声道采样缓冲区
 * @return 增强后的左右声道对
 */
QPair<QVector<double>, QVector<double>> StereoEnhance3::process(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    int N = qMin(left.size(), right.size());
    QPair<QVector<double>, QVector<double>> result;
    result.first.resize(N, 0.0);
    result.second.resize(N, 0.0);

    if (N == 0) {
        m_timeSum += timer.elapsed();
        return result;
    }

    /* 宽度系数: 将[0,1]映射到Side增益 */
    double sideGain = 2.0 * m_width;

    for (int i = 0; i < N; ++i) {
        /* M/S编码 */
        double mid = (left[i] + right[i]) * 0.5;
        double side = (left[i] - right[i]) * 0.5;

        /* 增强Side分量 */
        double enhancedSide = side * sideGain;

        /* M/S解码 */
        result.first[i] = mid - enhancedSide;   /* 左声道 */
        result.second[i] = mid + enhancedSide;   /* 右声道 */

        /* 软限幅防止削波 */
        result.first[i] = qBound(-1.0, result.first[i], 1.0);
        result.second[i] = qBound(-1.0, result.second[i], 1.0);
    }

    m_stats.totalSamplesProcessed += N * 2;
    m_stats.totalFramesApplied++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesApplied);

    emit enhanceApplied(N, m_width);

    return result;
}

/**
 * @brief 设置增强宽度
 *
 * 宽度参数控制立体声声场的扩展程度:
 * - 0.0: 完全单声道(Mid only)
 * - 0.5: 原始立体声(无增强)
 * - 1.0: 最大宽度(Side增益翻倍)
 *
 * @param width 宽度参数(0~1)
 */
void StereoEnhance3::setWidth(double width)
{
    QElapsedTimer timer;
    timer.start();

    m_width = qBound(0.0, width, 1.0);

    m_timeSum += timer.elapsed();
}

/**
 * @brief 计算当前立体声的相关系数
 *
 * 相关系数表示左右声道的线性相关程度:
 * r = Σ(L[i]*R[i]) / sqrt(ΣL[i]^2 * ΣR[i]^2)
 *
 * r接近1: 高度相关(窄立体声)
 * r接近0: 不相关(宽立体声)
 * r接近-1: 反相关(超宽/异常)
 *
 * @param left 左声道采样
 * @param right 右声道采样
 * @return 相关系数(-1~1)
 */
double StereoEnhance3::correlation(const QVector<double>& left,
                                    const QVector<double>& right) const
{
    int N = qMin(left.size(), right.size());
    if (N == 0) return 0.0;

    double sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;
    for (int i = 0; i < N; ++i) {
        sumLR += left[i] * right[i];
        sumLL += left[i] * left[i];
        sumRR += right[i] * right[i];
    }

    double denom = qSqrt(sumLL * sumRR);
    if (denom < 1e-15) return 0.0;
    return sumLR / denom;
}

/**
 * @brief 获取当前宽度设置
 * @return 宽度参数(0~1)
 */
double StereoEnhance3::width() const
{
    return m_width;
}

/**
 * @brief 重置所有统计数据
 *
 * 将采样计数、帧计数和计时归零。
 */
void StereoEnhance3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
