/**
 * @file Chromagram2.cpp
 * @brief 色度图2实现 — 和弦检测+调性估计
 *
 * 色度图将音频频谱映射到12个音高类别(C, C#, D, ..., B):
 * - 构建音符滤波器组(每个半音对应的频率bin)
 * - 从频谱中提取色度向量
 * - 和弦检测: 与和弦模板进行余弦相似度匹配
 * - 调性估计: Krumhansl-Schmuckler算法
 *
 * 统计信息跟踪: 计算次数、帧数、平均耗时。
 */

#include "utils/signal45/Chromagram2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
Chromagram2::Chromagram2(QObject* parent)
    : QObject(parent)
{
    buildNoteFilters();
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void Chromagram2::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_initialized = false;
    buildNoteFilters();
}

/**
 * @brief 设置FFT大小
 * @param fftSize FFT大小
 */
void Chromagram2::setFFTSize(int fftSize)
{
    m_fftSize = qMax(64, fftSize);
    m_initialized = false;
    buildNoteFilters();
}

/**
 * @brief 设置帧移大小
 * @param hopSize 帧移采样数
 */
void Chromagram2::setHopSize(int hopSize)
{
    m_hopSize = qMax(1, hopSize);
}

/**
 * @brief 构建音符滤波器组
 *
 * 对每个半音(0-11)和多个八度，找到FFT对应的频率bin。
 * 标准调音: A4 = 440Hz, C0 = ~16.35Hz。
 * 滤波器将附近bin的能量汇总到对应的色度bin。
 */
void Chromagram2::buildNoteFilters()
{
    const int numOctaves = 8;  ///< 覆盖C1~B8
    m_noteFilters.resize(12);

    // 标准频率: C0 = 440 * 2^(-57/12) ≈ 16.35 Hz
    const double c0 = 440.0 * qPow(2.0, -57.0 / 12.0);

    for (int note = 0; note < 12; ++note) {
        m_noteFilters[note].resize(m_fftSize / 2 + 1);

        for (int octave = 0; octave < numOctaves; ++octave) {
            // 音符的频率
            double freq = c0 * qPow(2.0, (octave * 12 + note) / 12.0);

            // 对应的FFT bin
            double bin = freq * m_fftSize / m_sampleRate;
            int binLow = qMax(0, static_cast<int>(qFloor(bin - 0.5)));
            int binHigh = qMin(m_fftSize / 2, static_cast<int>(qCeil(bin + 0.5)));

            // 在附近bin上设置高斯权重
            for (int b = binLow; b <= binHigh; ++b) {
                double dist = qAbs(b - bin);
                double weight = qExp(-0.5 * dist * dist);
                m_noteFilters[note][b] += weight;
            }
        }
    }

    // 归一化每个滤波器
    for (int note = 0; note < 12; ++note) {
        double sum = 0.0;
        for (double v : m_noteFilters[note]) sum += v;
        if (sum > 0.0) {
            for (double& v : m_noteFilters[note]) v /= sum;
        }
    }

    m_initialized = true;
}

/**
 * @brief 从单帧频谱计算色度向量
 *
 * @param frame 时域音频帧(将内部计算幅度谱)
 * @return 12维色度向量
 */
QVector<double> Chromagram2::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) {
        buildNoteFilters();
    }

    // 计算幅度谱
    const int N = qMin(frame.size(), m_fftSize);
    QVector<double> magnitude(m_fftSize / 2 + 1, 0.0);

    for (int k = 0; k <= m_fftSize / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / m_fftSize;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }

    // 应用滤波器组
    QVector<double> chroma(12, 0.0);
    for (int note = 0; note < 12; ++note) {
        double energy = 0.0;
        for (int k = 0; k < m_fftSize / 2 + 1; ++k) {
            energy += m_noteFilters[note][k] * magnitude[k];
        }
        chroma[note] = energy;
    }

    // 归一化
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-10) {
        for (double& v : chroma) v /= maxVal;
    }

    // 更新统计信息
    m_stats.totalComputations++;
    m_stats.totalFrames++;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return chroma;
}

/**
 * @brief 对整段信号计算色度序列
 * @param signal 完整音频信号
 * @return 每帧的色度向量序列
 */
QVector<QVector<double>> Chromagram2::computeSequence(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) {
        return {};
    }

    QVector<QVector<double>> result;
    int frameCount = 0;

    for (int start = 0; start + m_fftSize <= signal.size(); start += m_hopSize) {
        QVector<double> frame(m_fftSize);
        for (int i = 0; i < m_fftSize; ++i) {
            frame[i] = signal[start + i];
        }
        result.append(compute(frame));
        frameCount++;
    }

    m_stats.totalFrames += frameCount;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return result;
}

/**
 * @brief 生成和弦模板向量
 *
 * 支持大调/小调三和弦和七和弦。
 * 和弦模板: 根音位置设为1，三度/五度/七度位置设为对应值。
 *
 * @param chord 和弦名称，如 "C", "Cm", "C7", "Cm7"
 * @return 12维模板向量
 */
QVector<double> Chromagram2::chordTemplate(const QString& chord) const
{
    // 根音映射
    QMap<QString, int> noteMap = {
        {"C", 0}, {"C#", 1}, {"Db", 1}, {"D", 2}, {"D#", 3}, {"Eb", 3},
        {"E", 4}, {"F", 5}, {"F#", 6}, {"Gb", 6}, {"G", 7}, {"G#", 8},
        {"Ab", 8}, {"A", 9}, {"A#", 10}, {"Bb", 10}, {"B", 11}
    };

    // 解析根音
    int root = 0;
    QString suffix;
    for (auto it = noteMap.begin(); it != noteMap.end(); ++it) {
        if (chord.startsWith(it.key())) {
            root = it.value();
            suffix = chord.mid(it.key().length());
            break;
        }
    }

    QVector<double> tmpl(12, 0.0);
    tmpl[root] = 1.0;

    if (suffix.isEmpty() || suffix == "maj") {
        // 大三和弦: 根音 + 大三度 + 纯五度
        tmpl[(root + 4) % 12] = 1.0;
        tmpl[(root + 7) % 12] = 1.0;
    } else if (suffix == "m" || suffix == "min") {
        // 小三和弦: 根音 + 小三度 + 纯五度
        tmpl[(root + 3) % 12] = 1.0;
        tmpl[(root + 7) % 12] = 1.0;
    } else if (suffix == "7" || suffix == "dom7") {
        // 属七和弦
        tmpl[(root + 4) % 12] = 1.0;
        tmpl[(root + 7) % 12] = 1.0;
        tmpl[(root + 10) % 12] = 1.0;
    } else if (suffix == "m7") {
        // 小七和弦
        tmpl[(root + 3) % 12] = 1.0;
        tmpl[(root + 7) % 12] = 1.0;
        tmpl[(root + 10) % 12] = 1.0;
    } else if (suffix == "dim") {
        // 减三和弦
        tmpl[(root + 3) % 12] = 1.0;
        tmpl[(root + 6) % 12] = 1.0;
    } else if (suffix == "aug") {
        // 增三和弦
        tmpl[(root + 4) % 12] = 1.0;
        tmpl[(root + 8) % 12] = 1.0;
    }

    return tmpl;
}

/**
 * @brief 检测和弦
 *
 * 与所有24个大调/小调和弦模板计算余弦相似度，
 * 返回匹配度最高的和弦名称。
 *
 * @param chroma 12维色度向量
 * @return 和弦名称(如 "C", "Am")
 */
QString Chromagram2::detectChord(const QVector<double>& chroma) const
{
    if (chroma.size() != 12) return "Unknown";

    // 所有调上的大调和小调和弦
    QStringList noteNames = {"C", "C#", "D", "D#", "E", "F",
                              "F#", "G", "G#", "A", "A#", "B"};

    double bestScore = -1.0;
    QString bestChord = "Unknown";

    for (int i = 0; i < 12; ++i) {
        // 大调和弦
        QString majorChord = noteNames[i];
        QVector<double> majorTmpl = chordTemplate(majorChord);
        double score = cosineSimilarity(chroma, majorTmpl);
        if (score > bestScore) {
            bestScore = score;
            bestChord = majorChord;
        }

        // 小调和弦
        QString minorChord = noteNames[i] + "m";
        QVector<double> minorTmpl = chordTemplate(minorChord);
        score = cosineSimilarity(chroma, minorTmpl);
        if (score > bestScore) {
            bestScore = score;
            bestChord = minorChord;
        }
    }

    return bestChord;
}

/**
 * @brief 使用Krumhansl-Schmuckler算法检测调性
 *
 * 将色度序列的平均值与每个调的音高分布模板做相关分析，
 * 相关性最高的调即为估计的调性。
 *
 * @param chromaSequence 色度向量序列
 * @return 调性名称(如 "C Major", "A Minor")
 */
QString Chromagram2::detectKey(const QVector<QVector<double>>& chromaSequence) const
{
    if (chromaSequence.isEmpty()) return "Unknown";

    // 计算平均色度向量
    QVector<double> avgChroma(12, 0.0);
    for (const auto& chroma : chromaSequence) {
        for (int i = 0; i < 12 && i < chroma.size(); ++i) {
            avgChroma[i] += chroma[i];
        }
    }
    double count = chromaSequence.size();
    for (double& v : avgChroma) v /= count;

    // Krumhansl-Kessler 大调分布模板
    static const double majorProfile[12] = {
        6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88
    };
    // 小调分布模板
    static const double minorProfile[12] = {
        6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17
    };

    QStringList noteNames = {"C", "C#", "D", "D#", "E", "F",
                              "F#", "G", "G#", "A", "A#", "B"};

    double bestCorr = -2.0;
    QString bestKey = "C Major";
    bool bestIsMajor = true;

    for (int shift = 0; shift < 12; ++shift) {
        // 旋转色度向量
        QVector<double> rotated(12);
        for (int i = 0; i < 12; ++i) {
            rotated[i] = avgChroma[(i + shift) % 12];
        }

        // 大调相关
        double corrM = pearsonCorrelation(rotated, majorProfile);
        if (corrM > bestCorr) {
            bestCorr = corrM;
            bestKey = noteNames[shift] + " Major";
            bestIsMajor = true;
        }

        // 小调相关
        double corrm = pearsonCorrelation(rotated, minorProfile);
        if (corrm > bestCorr) {
            bestCorr = corrm;
            bestKey = noteNames[shift] + " Minor";
            bestIsMajor = false;
        }
    }

    return bestKey;
}

/**
 * @brief 重置所有统计信息
 */
void Chromagram2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算余弦相似度(内部辅助)
 */
double Chromagram2::cosineSimilarity(const QVector<double>& a,
                                      const QVector<double>& b) const
{
    double dot = 0.0, normA = 0.0, normB = 0.0;
    int len = qMin(a.size(), b.size());
    for (int i = 0; i < len; ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    double denom = qSqrt(normA) * qSqrt(normB);
    return (denom > 1e-15) ? dot / denom : 0.0;
}

/**
 * @brief 计算Pearson相关系数(内部辅助)
 */
double Chromagram2::pearsonCorrelation(const QVector<double>& x,
                                        const double y[12]) const
{
    double meanX = 0.0, meanY = 0.0;
    for (int i = 0; i < 12; ++i) {
        meanX += x[i];
        meanY += y[i];
    }
    meanX /= 12.0;
    meanY /= 12.0;

    double cov = 0.0, varX = 0.0, varY = 0.0;
    for (int i = 0; i < 12; ++i) {
        double dx = x[i] - meanX;
        double dy = y[i] - meanY;
        cov += dx * dy;
        varX += dx * dx;
        varY += dy * dy;
    }

    double denom = qSqrt(varX) * qSqrt(varY);
    return (denom > 1e-15) ? cov / denom : 0.0;
}
