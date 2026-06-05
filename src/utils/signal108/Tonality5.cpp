#include "Tonality5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化音调性分析器
 * @param parent 父对象指针
 */
Tonality5::Tonality5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Tonality5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算音调性指数
 *
 * 音调性指数衡量信号中音调成分的强度。
 * 通过分析功率谱的峰值与背景噪声的对比来计算。
 * 值域[0.0, 1.0]，接近1.0表示强音调成分。
 *
 * @param powerSpectrum 功率谱密度
 * @param sampleRate 采样率(Hz)
 * @return 音调性指数 [0.0, 1.0]
 */
double Tonality5::computeTonalityIndex(const QVector<double>& powerSpectrum, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int n = powerSpectrum.size();
    if (n == 0) {
        emit analysisCompleted(0.0);
        return 0.0;
    }

    /* 计算功率谱的算术平均和几何平均 */
    double arithMean = 0.0;
    double logSum = 0.0;
    int validCount = 0;

    for (double p : powerSpectrum) {
        if (p > 0.0) {
            arithMean += p;
            logSum += qLn(p);
            validCount++;
        }
    }

    if (validCount == 0) {
        m_lastTonality = 0.0;
        m_stats.lastTonalityIndex = 0.0;
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.totalAnalyses++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;
        emit analysisCompleted(0.0);
        return 0.0;
    }

    arithMean /= validCount;
    double geoMean = qExp(logSum / validCount);

    /* 频谱平坦度的补数作为音调性指标 */
    double flatness = (arithMean > 0) ? geoMean / arithMean : 0.0;
    double tonality = 1.0 - flatness;
    tonality = qBound(0.0, tonality, 1.0);

    m_lastTonality = tonality;
    m_stats.lastTonalityIndex = tonality;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalyses++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(tonality);
    return tonality;
}

/**
 * @brief 检测主要音调成分
 *
 * 通过峰值检测算法找出功率谱中的显著峰值，
 * 每个峰值对应一个音调成分，按幅度降序排列。
 *
 * @param powerSpectrum 功率谱密度
 * @param sampleRate 采样率(Hz)
 * @param maxTones 最大检测音调数
 * @return 检测到的音调列表
 */
QVector<Tonality5::ToneInfo> Tonality5::detectTones(const QVector<double>& powerSpectrum,
                                                      double sampleRate, int maxTones)
{
    QElapsedTimer timer;
    timer.start();

    const int n = powerSpectrum.size();
    QVector<ToneInfo> tones;

    if (n == 0) {
        emit analysisCompleted(m_lastTonality);
        return tones;
    }

    /* 计算功率谱平均值作为噪声基准 */
    double avgPower = 0.0;
    for (double p : powerSpectrum) avgPower += p;
    avgPower /= n;

    /* 峰值检测 */
    QVector<int> peaks;
    for (int i = 1; i < n - 1; ++i) {
        if (powerSpectrum[i] > powerSpectrum[i - 1] &&
            powerSpectrum[i] > powerSpectrum[i + 1] &&
            powerSpectrum[i] > avgPower * 3.0) { /* 高于平均值3倍 */
            peaks.append(i);
        }
    }

    /* 按幅度降序排列 */
    std::sort(peaks.begin(), peaks.end(), [&powerSpectrum](int a, int b) {
        return powerSpectrum[a] > powerSpectrum[b];
    });

    /* 提取前maxTones个峰值信息 */
    int count = qMin(maxTones, peaks.size());
    for (int i = 0; i < count; ++i) {
        int bin = peaks[i];
        ToneInfo info;
        info.frequencyHz = bin * sampleRate / (2.0 * n);
        info.amplitudeDb = 10.0 * qLn(qMax(powerSpectrum[bin], 1e-10)) / qLn(10.0);
        info.prominence = powerSpectrum[bin] / qMax(avgPower, 1e-10);
        tones.append(info);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalyses++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(m_lastTonality);
    return tones;
}

/**
 * @brief 从时域信号直接分析音调性
 *
 * 先计算功率谱，再调用频域分析方法。
 *
 * @param samples 时域采样数据
 * @param sampleRate 采样率(Hz)
 * @return 音调性指数
 */
double Tonality5::analyzeFromTimeDomain(const QVector<double>& samples, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    if (n == 0) return 0.0;

    /* 计算功率谱 */
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

    return computeTonalityIndex(powerSpec, sampleRate);
}
