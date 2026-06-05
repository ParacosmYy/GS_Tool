/**
 * @file CepstralLifter.h
 * @brief 倒谱提升器 — 语音处理中的倒谱加权与滤波
 *
 * 功能: 对倒谱系数施加提升窗函数，增强或抑制特定
 *       quefrency 区间。支持低通/高通倒谱滤波，用于
 *       共振峰提取、基频估计和声道特征分离。
 *
 * 协作: CepstralAnalysis(倒谱计算) / MelFrequencyCepstrum(MFCC特征)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 倒谱提升器 — 对倒谱系数施加加权窗
 */
class CepstralLifter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalApplied = 0;          ///< 累计提升次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param numCoeffs 倒谱系数数量
     * @param lifterCoeff 提升系数L(通常22~26)
     * @param parent 父对象
     */
    explicit CepstralLifter(int numCoeffs = 13, int lifterCoeff = 22,
                             QObject* parent = nullptr);

    /**
     * @brief 应用标准倒谱提升窗
     * @param cepstrum 输入倒谱系数
     * @return 提升后的倒谱系数
     *
     * 公式: y[i] = (1 + L/2 * sin(pi*i/L)) * x[i]
     */
    QVector<double> apply(const QVector<double>& cepstrum);

    /**
     * @brief 低通倒谱滤波 — 保留低quefrency分量(声道信息)
     * @param cepstrum 输入倒谱
     * @param cutoff 截止quefrency索引
     * @return 滤波后的倒谱
     */
    QVector<double> lowPassLifter(const QVector<double>& cepstrum, int cutoff);

    /**
     * @brief 高通倒谱滤波 — 保留高quefrency分量(激励信息)
     * @param cepstrum 输入倒谱
     * @param cutoff 截止quefrency索引
     * @return 滤波后的倒谱
     */
    QVector<double> highPassLifter(const QVector<double>& cepstrum, int cutoff);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 获取系数数量 */
    int numCoeffs() const { return m_numCoeffs; }

    /** @brief 获取提升系数 */
    int lifterCoeff() const { return m_lifterCoeff; }

    /** @brief 设置系数数量 */
    void setNumCoeffs(int n);

    /** @brief 设置提升系数 */
    void setLifterCoeff(int L);

signals:
    /** @brief 提升完成 @param coeffs 输出系数数量 */
    void lifterApplied(int coeffs);

private:
    int    m_numCoeffs;    ///< 倒谱系数数量
    int    m_lifterCoeff;  ///< 提升系数L
    double m_timeSum;      ///< 处理时间累加器
    Stats  m_stats;        ///< 统计信息
};
