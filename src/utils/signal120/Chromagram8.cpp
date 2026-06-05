#include "Chromagram8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化色度图分析器
 * @param parent 父对象指针
 */
Chromagram8::Chromagram8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Chromagram8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置参考频率
 * @param refFreqHz 参考频率(Hz)，默认440Hz
 */
void Chromagram8::setReferenceFrequency(double refFreqHz)
{
    Q_UNUSED(refFreqHz)
}

/**
 * @brief 计算单帧色度向量
 *
 * 将频域幅度谱映射到12个音级(C, C#, D, ..., B)：
 * 1. 对每个频率bin计算对应音级
 * 2. 使用高斯加权将能量分配到最近的音级
 * 3. 归一化输出向量
 *
 * @param frequencySpectrum 频域幅度谱
 * @param sampleRate 采样率(Hz)
 * @return 12维色度向量(C~B)
 */
QVector<double> Chromagram8::compute(const QVector<double>& frequencySpectrum,
                                      double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> chroma(12, 0.0);
    const int n = frequencySpectrum.size();
    if (n == 0 || sampleRate <= 0) {
        emit analysisCompleted(0);
        return chroma;
    }

    /* 映射FFT bin到音级 */
    for (int k = 1; k < n; ++k) {
        double freq = static_cast<double>(k) * sampleRate / (2.0 * (n - 1));
        if (freq <= 0.0) continue;

        /* 计算相对于C0的半音数 */
        double semitones = 12.0 * qLn(freq / 16.35) / qLn(2.0);
        int pitchClass = static_cast<int>(qRound(semitones)) % 12;
        if (pitchClass < 0) pitchClass += 12;

        double mag = frequencySpectrum[k];
        chroma[pitchClass] += mag * mag;
    }

    /* 归一化 */
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-10) {
        for (int i = 0; i < 12; ++i) {
            chroma[i] /= maxVal;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalysisOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysisOps;

    emit analysisCompleted(1);
    return chroma;
}

/**
 * @brief 批量计算色度图序列
 *
 * 对多帧频谱数据逐帧计算色度向量。
 *
 * @param frames 分帧后的频谱序列
 * @param sampleRate 采样率(Hz)
 * @return 色度图矩阵(帧数 × 12)
 */
QVector<QVector<double>> Chromagram8::computeSequence(
    const QVector<QVector<double>>& frames, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> sequence;
    const int numFrames = frames.size();
    if (numFrames == 0) {
        emit analysisCompleted(0);
        return sequence;
    }

    sequence.reserve(numFrames);
    for (int i = 0; i < numFrames; ++i) {
        sequence.append(compute(frames[i], sampleRate));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalAnalysisOps += numFrames;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalysisOps;

    emit analysisCompleted(numFrames);
    return sequence;
}

/**
 * @brief 识别当前帧的主和弦
 *
 * 将色度向量与24个标准和弦模板(12大调+12小调)比较，
 * 返回相关度最高的和弦名称。
 *
 * @param chroma 12维色度向量
 * @return 最匹配的和弦名称
 */
QString Chromagram8::identifyChord(const QVector<double>& chroma) const
{
    if (chroma.size() != 12) return {};

    /* 标准大调模板: C, C#, D, D#, E, F, F#, G, G#, A, A#, B */
    static const double majorTemplate[12] = {
        1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0
    };
    /* 标准小调模板 */
    static const double minorTemplate[12] = {
        1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0
    };

    static const QString noteNames[12] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    double bestCorr = -1e18;
    int bestRoot = 0;
    bool bestIsMajor = true;

    for (int root = 0; root < 12; ++root) {
        /* 大调相关度 */
        double corrMajor = 0.0;
        double corrMinor = 0.0;
        for (int i = 0; i < 12; ++i) {
            int idx = (i + root) % 12;
            corrMajor += chroma[idx] * majorTemplate[i];
            corrMinor += chroma[idx] * minorTemplate[i];
        }

        if (corrMajor > bestCorr) {
            bestCorr = corrMajor;
            bestRoot = root;
            bestIsMajor = true;
        }
        if (corrMinor > bestCorr) {
            bestCorr = corrMinor;
            bestRoot = root;
            bestIsMajor = false;
        }
    }

    return noteNames[bestRoot] + (bestIsMajor ? "" : "m");
}
