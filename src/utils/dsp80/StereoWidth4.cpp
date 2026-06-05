/**
 * @file StereoWidth4.cpp
 * @brief 立体声宽度控制器实现 — M/S编解码 + 宽度调节 + 相位一致性检测
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 实现基于中间/侧边(Mid/Side)处理的立体声宽度控制器:
 * 1. L/R -> M/S 编码: M = (L+R)/2, S = (L-R)/2
 * 2. 通过调节Side增益控制立体声宽度
 * 3. M/S -> L/R 解码: L = M + S*gain, R = M - S*gain
 * 4. 检测反相问题并发出警告信号
 * 5. 计算声道间互相关系数
 */

#include "utils/dsp80/StereoWidth4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 常量定义
// ──────────────────────────────────────────────

/** @brief 反相检测阈值 — 相关系数低于此值触发警告 */
static constexpr double kPhaseWarningThreshold = -0.5;

/** @brief 最大允许宽度因子 */
static constexpr double kMaxWidth = 3.0;

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化立体声宽度控制器
 * @param parent 父QObject对象
 *
 * 默认宽度: 1.0 (原始立体声，不做修改)
 */
StereoWidth4::StereoWidth4(QObject* parent)
    : QObject(parent)
    , m_width(1.0)
{
    setObjectName(QStringLiteral("StereoWidth4"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置宽度因子
 *
 * 通过M/S处理控制立体声感知宽度:
 * - width = 0.0: 完全单声道(Side=0)
 * - width = 1.0: 原始立体声(不修改)
 * - width > 1.0: 增强立体声(Side放大)
 * - width = 2.0: Side加倍，极限扩展
 *
 * 内部限制范围 [0.0, 3.0] 防止过度处理。
 *
 * @param width 宽度因子
 */
void StereoWidth4::setWidth(double width)
{
    m_width = qBound(0.0, width, kMaxWidth);
}

// ──────────────────────────────────────────────
// 核心处理
// ──────────────────────────────────────────────

/**
 * @brief 处理立体声帧(左右声道)
 *
 * 完整处理流程:
 * 1. 验证输入: 左右声道长度必须相同且非空
 * 2. L/R -> M/S 编码
 * 3. 对Side通道施加宽度增益
 * 4. 自动平衡: 增加宽度时微调Mid增益，保持总能量
 * 5. M/S -> L/R 解码
 * 6. 反相检测: 采样级检查是否有反相片段
 * 7. 输出软限幅防止削波
 *
 * @param left 左声道采样数据
 * @param right 右声道采样数据
 * @return 处理后的左右声道对 (QPair<左, 右>)
 */
QPair<QVector<double>, QVector<double>> StereoWidth4::process(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    if (left.isEmpty() || right.isEmpty() || left.size() != right.size()) {
        m_stats.totalFramesProcessed++;
        const double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;
        return {};
    }

    const int n = left.size();
    QVector<double> outL(n, 0.0);
    QVector<double> outR(n, 0.0);

    // 计算能量平衡增益
    // width > 1时Mid通道轻微降低，保持总能量近似恒定
    double midGain = 1.0;
    if (m_width > 1.0) {
        midGain = 1.0 / qSqrt(m_width);
    }
    double sideGain = m_width;

    for (int i = 0; i < n; ++i) {
        double L = left[i];
        double R = right[i];

        // ── M/S 编码 ──
        double M = (L + R) * 0.5 * midGain;  // 中间信号(中心声像)
        double S = (L - R) * 0.5 * sideGain;  // 侧边信号(立体声差异)

        // ── M/S 解码 ──
        outL[i] = M + S;
        outR[i] = M - S;

        // ── 软限幅(防止宽度扩展导致削波) ──
        outL[i] = softClipSample(outL[i]);
        outR[i] = softClipSample(outR[i]);
    }

    // ── 反相检测 ──
    // 逐段检查左右声道相关系数，发现反相片段时发射警告
    int segmentSize = qMax(1, n / 8); // 将帧分为8段检测
    for (int seg = 0; seg < 8; ++seg) {
        int start = seg * segmentSize;
        int end = qMin(start + segmentSize, n);
        if (end <= start) break;

        double sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;
        for (int i = start; i < end; ++i) {
            sumLR += outL[i] * outR[i];
            sumLL += outL[i] * outL[i];
            sumRR += outR[i] * outR[i];
        }

        double denom = qSqrt(sumLL * sumRR);
        if (denom > 1e-15) {
            double corr = sumLR / denom;
            if (corr < kPhaseWarningThreshold) {
                m_stats.totalPhaseWarnings++;
                int warnIndex = (start + end) / 2;
                emit phaseWarningDetected(warnIndex);
            }
        }
    }

    // ── 更新统计 ──
    m_stats.totalFramesProcessed++;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    return qMakePair(outL, outR);
}

// ──────────────────────────────────────────────
// 分析方法
// ──────────────────────────────────────────────

/**
 * @brief 计算当前帧的立体声相关系数
 *
 * 互相关系数 rho 的含义:
 * - rho = +1.0: 完全相关(L=R，纯单声道)
 * - rho = 0.0: 不相关(独立信号，最大立体感)
 * - rho = -1.0: 完全反相(L=-R，相位相反)
 *
 * 公式: rho = sum(L*R) / sqrt(sum(L^2) * sum(R^2))
 *
 * @param left 左声道
 * @param right 右声道
 * @return 互相关系数 [-1.0, 1.0]
 */
double StereoWidth4::correlation(const QVector<double>& left,
                                  const QVector<double>& right) const
{
    if (left.isEmpty() || right.isEmpty() || left.size() != right.size()) {
        return 0.0;
    }

    const int n = left.size();
    double sumLR = 0.0;
    double sumLL = 0.0;
    double sumRR = 0.0;

    for (int i = 0; i < n; ++i) {
        sumLR += left[i] * right[i];
        sumLL += left[i] * left[i];
        sumRR += right[i] * right[i];
    }

    double denom = qSqrt(sumLL * sumRR);
    if (denom < 1e-15) return 0.0;

    double rho = sumLR / denom;
    return qBound(-1.0, rho, 1.0);
}

/**
 * @brief 获取M/S编码后的中间和侧边信号
 *
 * 将左右声道转换为Mid/Side表示:
 * - Mid = (L + R) / 2 (中心声像)
 * - Side = (L - R) / 2 (立体声差异)
 *
 * M/S编码是可逆的: L = Mid + Side, R = Mid - Side
 *
 * @param left 左声道
 * @param right 右声道
 * @return QPair<中间信号, 侧边信号>
 */
QPair<QVector<double>, QVector<double>> StereoWidth4::toMS(
    const QVector<double>& left, const QVector<double>& right) const
{
    if (left.isEmpty() || right.isEmpty() || left.size() != right.size()) {
        return {};
    }

    const int n = left.size();
    QVector<double> mid(n, 0.0);
    QVector<double> side(n, 0.0);

    for (int i = 0; i < n; ++i) {
        mid[i]  = (left[i] + right[i]) * 0.5;
        side[i] = (left[i] - right[i]) * 0.5;
    }

    return qMakePair(mid, side);
}

/**
 * @brief 重置所有累计统计信息
 */
void StereoWidth4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 软限幅
// ──────────────────────────────────────────────

/**
 * @brief 单采样软限幅
 *
 * 使用双曲正切近似实现平滑限幅。
 * 在 ±1.0 范围内线性通过，超出后渐进压缩。
 * 避免硬削波带来的数字失真。
 *
 * @param x 输入采样值
 * @return 限幅后的采样值
 */
double StereoWidth4::softClipSample(double x) const
{
    if (qAbs(x) <= 1.0) return x;

    // 三次多项式软限幅
    double sign = (x > 0) ? 1.0 : -1.0;
    double absX = qAbs(x);

    if (absX > 2.0) {
        return sign * 1.5;
    }

    // y = 1.5 - 0.5 * (2 - |x|)^2
    double t = 2.0 - absX;
    return sign * (1.5 - 0.5 * t * t);
}
