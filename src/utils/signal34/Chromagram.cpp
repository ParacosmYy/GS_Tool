/**
 * @file Chromagram.cpp
 * @brief 色度图分析实现 — 色度特征/调性估计/和弦检测
 */

#include "utils/signal34/Chromagram.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Chromagram::Chromagram(QObject* parent)
    : QObject(parent)
{
    createChromaMap();
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void Chromagram::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    createChromaMap();
}

/** @brief 设置FFT大小 @param size FFT大小 */
void Chromagram::setFFTSize(int size)
{
    int p = 1;
    while (p < size) p <<= 1;
    m_fftSize = qMax(64, p);
    createChromaMap();
}

/** @brief 设置跳跃大小 @param hop 跳跃采样数 */
void Chromagram::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/** @brief 设置参考频率(A4) @param freq 参考频率(Hz) */
void Chromagram::setReferenceFreq(double freq)
{
    m_refFreq = qMax(1.0, freq);
    createChromaMap();
}

/**
 * @brief 计算单帧色度特征
 * @param frame 输入帧(FFT大小)
 * @return 12维色度向量(C, C#, D, ..., B)
 */
QVector<double> Chromagram::computeFrame(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int halfN = qMin(m_fftSize / 2 + 1, frame.size());
    QVector<double> chroma(12, 0.0);

    /* 对每个FFT bin计算其所属的色度类别 */
    for (int i = 1; i < halfN; ++i) {
        double freq = static_cast<double>(i) * m_sampleRate
            / static_cast<double>(m_fftSize);

        if (freq < 20.0 || freq > m_sampleRate / 2.0) continue;

        /* 将频率映射到色度: pitch class = round(12 * log2(f / C0)) % 12 */
        double semitone = 12.0 * qLn(freq / m_refFreq) / qLn(2.0) + 9.0;
        int pitchClass = static_cast<int>(qRound(semitone)) % 12;
        if (pitchClass < 0) pitchClass += 12;

        double magnitude = qFabs(frame[i]);
        chroma[pitchClass] += magnitude * magnitude;
    }

    /* 归一化 */
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-10) {
        for (int i = 0; i < 12; ++i) {
            chroma[i] /= maxVal;
        }
    }

    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalFrames));

    emit frameComputed(m_stats.totalFrames);
    return chroma;
}

/**
 * @brief 计算整段音频的色度图
 * @param audio 音频采样序列
 * @return 色度图(每帧12维)
 */
QVector<QVector<double>> Chromagram::compute(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    if (audio.size() < m_fftSize) return {};

    int numFrames = (audio.size() - m_fftSize) / m_hopSize + 1;
    QVector<QVector<double>> result;
    result.reserve(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 加窗FFT */
        QVector<double> real(m_fftSize, 0.0);
        QVector<double> imag(m_fftSize, 0.0);

        for (int i = 0; i < m_fftSize; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
            real[i] = audio[start + i] * w;
        }

        forwardFFT(real, imag);

        /* 提取幅度谱 */
        int halfN = m_fftSize / 2 + 1;
        QVector<double> magnitudes(halfN);
        for (int i = 0; i < halfN; ++i) {
            magnitudes[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
        }

        /* 计算色度 */
        result.append(computeFrame(magnitudes));
    }

    m_timeSum += timer.elapsed();
    return result;
}

/**
 * @brief 从色度向量估计调性(Key)
 * @param chroma 12维色度向量
 * @return 调性编号(0=C major, 1=C# major, ..., 11=B major,
 *         12=C minor, ..., 23=B minor)
 */
int Chromagram::estimateKey(const QVector<double>& chroma) const
{
    if (chroma.size() != 12) return 0;

    /* Krumhansl-Schmuckler调性估计 */
    /* 大调模板 */
    static const double majorProfile[12] = {
        6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88
    };

    /* 小调模板 */
    static const double minorProfile[12] = {
        6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17
    };

    double bestCorr = -2.0;
    int bestKey = 0;

    /* 对每种调性计算与模板的相关系数 */
    for (int shift = 0; shift < 12; ++shift) {
        /* 大调 */
        double corr = pearsonCorr(chroma, majorProfile, shift);
        if (corr > bestCorr) {
            bestCorr = corr;
            bestKey = shift;
        }

        /* 小调 */
        corr = pearsonCorr(chroma, minorProfile, shift);
        if (corr > bestCorr) {
            bestCorr = corr;
            bestKey = shift + 12;
        }
    }

    return bestKey;
}

/**
 * @brief 和弦检测
 * @param chroma 12维色度向量
 * @return 候选和弦列表(相关度, 和弦编号)
 */
QVector<QPair<double, int>> Chromagram::chordDetect(const QVector<double>& chroma) const
{
    if (chroma.size() != 12) return {};

    /* 定义和弦模板: 大三和弦、小三和弦、属七和弦、减三和弦 */
    struct ChordTemplate {
        int notes[4];
        int noteCount;
        int baseIndex;
    };

    /* 大三和弦: 根音-大三度-纯五度 */
    static const int majorNotes[3] = {0, 4, 7};
    /* 小三和弦: 根音-小三度-纯五度 */
    static const int minorNotes[3] = {0, 3, 7};
    /* 属七和弦: 根音-大三度-纯五度-小七度 */
    static const int dom7Notes[4] = {0, 4, 7, 10};
    /* 减三和弦: 根音-小三度-减五度 */
    static const int dimNotes[3] = {0, 3, 6};

    QVector<QPair<double, int>> results;
    results.reserve(48);

    /* 对每个根音(0-11)分别匹配四种和弦 */
    for (int root = 0; root < 12; ++root) {
        /* 大三和弦 */
        double score = chordMatch(chroma, majorNotes, 3, root);
        results.append(qMakePair(score, root * 4 + 0));

        /* 小三和弦 */
        score = chordMatch(chroma, minorNotes, 3, root);
        results.append(qMakePair(score, root * 4 + 1));

        /* 属七和弦 */
        score = chordMatch(chroma, dom7Notes, 4, root);
        results.append(qMakePair(score, root * 4 + 2));

        /* 减三和弦 */
        score = chordMatch(chroma, dimNotes, 3, root);
        results.append(qMakePair(score, root * 4 + 3));
    }

    /* 按匹配度降序排序 */
    std::sort(results.begin(), results.end(),
        [](const QPair<double, int>& a, const QPair<double, int>& b) {
            return a.first > b.first;
        });

    return results;
}

/** @brief 重置统计 */
void Chromagram::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 创建色度映射表(FFT bin到色度类别的映射)
 */
void Chromagram::createChromaMap()
{
    m_chromaMap.resize(12);
    for (int c = 0; c < 12; ++c) {
        m_chromaMap[c].clear();
    }

    int halfN = m_fftSize / 2 + 1;
    for (int i = 1; i < halfN; ++i) {
        double freq = static_cast<double>(i) * m_sampleRate
            / static_cast<double>(m_fftSize);
        if (freq < 20.0) continue;

        double semitone = 12.0 * qLn(freq / m_refFreq) / qLn(2.0) + 9.0;
        int pitchClass = static_cast<int>(qRound(semitone)) % 12;
        if (pitchClass < 0) pitchClass += 12;

        if (pitchClass < 12) {
            m_chromaMap[pitchClass].append(i);
        }
    }
}

/**
 * @brief 计算Pearson相关系数(带循环移位)
 * @param chroma 色度向量
 * @param profile 调性模板
 * @param shift 循环移位量
 * @return 相关系数
 */
double Chromagram::pearsonCorr(const QVector<double>& chroma,
    const double profile[12], int shift) const
{
    double sumA = 0.0, sumB = 0.0, sumAB = 0.0;
    double sumA2 = 0.0, sumB2 = 0.0;

    for (int i = 0; i < 12; ++i) {
        int idx = (i + shift) % 12;
        double a = chroma[idx];
        double b = profile[i];

        sumA += a;
        sumB += b;
        sumAB += a * b;
        sumA2 += a * a;
        sumB2 += b * b;
    }

    double n = 12.0;
    double num = sumAB - sumA * sumB / n;
    double den = qSqrt((sumA2 - sumA * sumA / n) * (sumB2 - sumB * sumB / n));

    return (den > 1e-10) ? num / den : 0.0;
}

/**
 * @brief 和弦模板匹配得分
 * @param chroma 色度向量
 * @param notes 和弦音程
 * @param noteCount 音数
 * @param root 根音偏移
 * @return 匹配分数[0, 1]
 */
double Chromagram::chordMatch(const QVector<double>& chroma,
    const int* notes, int noteCount, int root) const
{
    double match = 0.0;
    double total = 0.0;

    for (int i = 0; i < 12; ++i) {
        total += chroma[i];
    }
    if (total < 1e-10) return 0.0;

    /* 计算和弦音的能量占比 */
    double chordEnergy = 0.0;
    for (int n = 0; n < noteCount; ++n) {
        int idx = (root + notes[n]) % 12;
        chordEnergy += chroma[idx];
    }

    /* 扣除非和弦音的能量 */
    double nonChordEnergy = total - chordEnergy;
    match = (chordEnergy - 0.5 * nonChordEnergy) / total;

    return qBound(0.0, match, 1.0);
}

/**
 * @brief 原地FFT(Cooley-Tukey)
 * @param real 实部数组
 * @param imag 虚部数组
 */
void Chromagram::forwardFFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    if (N <= 1) return;

    int bits = 0;
    for (int tmp = N; tmp > 1; tmp >>= 1) ++bits;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) j |= (1 << (bits - 1 - b));
        }
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle);
        double wI = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nr;
            }
        }
    }
}
