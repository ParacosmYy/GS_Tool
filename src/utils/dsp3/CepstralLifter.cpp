/**
 * @file CepstralLifter.cpp
 * @brief 倒谱提升器实现 — 标准提升窗 + 低通/高通倒谱滤波
 */

#include "utils/dsp3/CepstralLifter.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数
 * @param numCoeffs 倒谱系数数量
 * @param lifterCoeff 提升系数L
 * @param parent 父对象
 */
CepstralLifter::CepstralLifter(int numCoeffs, int lifterCoeff,
                                 QObject* parent)
    : QObject(parent)
    , m_numCoeffs(qMax(1, numCoeffs))
    , m_lifterCoeff(qMax(1, lifterCoeff))
    , m_timeSum(0.0)
{
}

/**
 * @brief 应用标准倒谱提升窗
 * @param cepstrum 输入倒谱系数
 * @return 提升后的倒谱系数
 *
 * 对每个系数施加 sin 窗加权，使高阶系数获得更大增益，
 * 补偿倒谱分析中高阶分量的衰减。
 */
QVector<double> CepstralLifter::apply(const QVector<double>& cepstrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int n = qMin(cepstrum.size(), m_numCoeffs);
    result.reserve(n);

    for (int i = 0; i < n; ++i) {
        /* 标准 sin 提升窗: w[i] = 1 + L/2 * sin(pi * i / L) */
        double w = 1.0 + (m_lifterCoeff / 2.0)
            * qSin(M_PI * static_cast<double>(i)
                   / static_cast<double>(m_lifterCoeff));
        result.append(cepstrum[i] * w);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalApplied;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalApplied);

    emit lifterApplied(result.size());
    return result;
}

/**
 * @brief 低通倒谱滤波 — 保留低quefrency分量
 * @param cepstrum 输入倒谱
 * @param cutoff 截止quefrency索引
 * @return 滤波后的倒谱
 *
 * 保留 [0, cutoff] 范围内的倒谱系数，其余置零。
 * 低quefrency对应频谱包络(声道传递函数)。
 */
QVector<double> CepstralLifter::lowPassLifter(
    const QVector<double>& cepstrum, int cutoff)
{
    QElapsedTimer timer;
    timer.start();

    int n = cepstrum.size();
    cutoff = qBound(0, cutoff, n - 1);

    QVector<double> result(n, 0.0);

    /* 使用余弦平滑过渡避免吉布斯现象 */
    for (int i = 0; i <= cutoff; ++i) {
        double w = 1.0;
        /* 在截止点附近施加余弦滚降(最后20%区间) */
        int rampStart = qMax(0, static_cast<int>(cutoff * 0.8));
        if (i > rampStart && cutoff > rampStart) {
            double t = static_cast<double>(i - rampStart)
                / static_cast<double>(cutoff - rampStart);
            w = 0.5 * (1.0 + qCos(M_PI * t));
        }
        result[i] = cepstrum[i] * w;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalApplied;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalApplied);

    emit lifterApplied(n);
    return result;
}

/**
 * @brief 高通倒谱滤波 — 保留高quefrency分量
 * @param cepstrum 输入倒谱
 * @param cutoff 截止quefrency索引
 * @return 滤波后的倒谱
 *
 * 保留 [cutoff, n) 范围内的倒谱系数，其余置零。
 * 高quefrency对应频谱精细结构(基频和谐波)。
 */
QVector<double> CepstralLifter::highPassLifter(
    const QVector<double>& cepstrum, int cutoff)
{
    QElapsedTimer timer;
    timer.start();

    int n = cepstrum.size();
    cutoff = qBound(0, cutoff, n - 1);

    QVector<double> result(n, 0.0);

    /* 余弦平滑过渡避免吉布斯现象 */
    for (int i = cutoff; i < n; ++i) {
        double w = 1.0;
        /* 在截止点附近施加余弦上升(前20%区间) */
        int rampEnd = qMin(n - 1,
            cutoff + static_cast<int>((n - cutoff) * 0.2));
        if (i <= rampEnd && rampEnd > cutoff) {
            double t = static_cast<double>(i - cutoff)
                / static_cast<double>(rampEnd - cutoff);
            w = 0.5 * (1.0 - qCos(M_PI * t));
        }
        result[i] = cepstrum[i] * w;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalApplied;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalApplied);

    emit lifterApplied(n);
    return result;
}

/** @brief 重置统计 */
void CepstralLifter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 设置系数数量 @param n 新的系数数量 */
void CepstralLifter::setNumCoeffs(int n)
{
    m_numCoeffs = qMax(1, n);
}

/** @brief 设置提升系数 @param L 新的提升系数 */
void CepstralLifter::setLifterCoeff(int L)
{
    m_lifterCoeff = qMax(1, L);
}
