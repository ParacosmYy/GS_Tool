#include "SpectralFlatness5.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化频谱平坦度分析器
 * @param parent 父对象指针
 */
SpectralFlatness5::SpectralFlatness5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SpectralFlatness5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 从功率谱计算频谱平坦度
 *
 * 频谱平坦度(Spectral Flatness)定义为几何平均与算术平均之比：
 * SF = geometric_mean(P) / arithmetic_mean(P)
 * SF接近1.0表示白噪声(平坦频谱)，接近0.0表示音调信号(尖峰频谱)。
 *
 * @param powerSpectrum 功率谱密度(非负值)
 * @return 频谱平坦度 [0.0, 1.0]
 */
double SpectralFlatness5::compute(const QVector<double>& powerSpectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int n = powerSpectrum.size();
    if (n == 0) {
        emit analysisCompleted(0.0);
        return 0.0;
    }

    /* 过滤掉零值以避免对数计算问题 */
    QVector<double> valid;
    valid.reserve(n);
    for (double v : powerSpectrum) {
        if (v > 0.0) valid.append(v);
    }

    if (valid.isEmpty()) {
        m_lastFlatness = 0.0;
        m_stats.lastFlatness = 0.0;

        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.totalAnalyses++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;
        emit analysisCompleted(0.0);
        return 0.0;
    }

    /* 使用对数计算几何平均避免溢出 */
    double logSum = 0.0;
    double arithmeticSum = 0.0;
    for (double v : valid) {
        logSum += qLn(v);
        arithmeticSum += v;
    }

    double geometricMean = qExp(logSum / valid.size());
    double arithmeticMean = arithmeticSum / valid.size();

    double flatness = (arithmeticMean > 0.0) ? geometricMean / arithmeticMean : 0.0;
    flatness = qBound(0.0, flatness, 1.0);

    m_lastFlatness = flatness;
    m_stats.lastFlatness = flatness;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalyses++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(flatness);
    return flatness;
}

/**
 * @brief 从时域采样直接计算频谱平坦度
 *
 * 先通过DFT将时域信号转换为功率谱，再计算频谱平坦度。
 *
 * @param samples 时域采样数据
 * @return 频谱平坦度
 */
double SpectralFlatness5::computeFromTimeDomain(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    if (n == 0) return 0.0;

    /* 计算功率谱（简化DFT） */
    QVector<double> powerSpec(n / 2 + 1);
    for (int k = 0; k <= n / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += samples[i] * qCos(angle);
            im -= samples[i] * qSin(angle);
        }
        powerSpec[k] = (re * re + im * im) / (n * n);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    /* 委托给功率谱计算方法 */
    return compute(powerSpec);
}

/**
 * @brief 计算指定频段的平坦度
 *
 * @param powerSpectrum 功率谱密度
 * @param startBin 起始频率bin索引
 * @param endBin 结束频率bin索引
 * @return 子带频谱平坦度
 */
double SpectralFlatness5::computeBand(const QVector<double>& powerSpectrum, int startBin, int endBin)
{
    if (startBin < 0) startBin = 0;
    if (endBin >= powerSpectrum.size()) endBin = powerSpectrum.size() - 1;
    if (startBin > endBin) return 0.0;

    QVector<double> subBand(powerSpectrum.begin() + startBin, powerSpectrum.begin() + endBin + 1);
    return compute(subBand);
}

/**
 * @brief 判断信号是否为类噪声
 * @param threshold 判定阈值
 * @return 是否为类噪声信号
 */
bool SpectralFlatness5::isNoiseLike(double threshold) const
{
    return m_lastFlatness >= threshold;
}
