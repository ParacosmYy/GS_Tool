#include "MelSpectrogram7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Mel频谱图引擎
 * @param parent 父对象指针
 */
MelSpectrogram7::MelSpectrogram7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MelSpectrogram7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置音频参数
 * @param sampleRate 采样率(Hz)
 * @param fftSize FFT窗口长度
 */
void MelSpectrogram7::setAudioParams(double sampleRate, int fftSize)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_fftSize = qMax(4, fftSize);
}

/**
 * @brief 设置Mel频带数量
 * @param bins Mel频带数
 */
void MelSpectrogram7::setMelBinCount(int bins)
{
    m_melBins = qMax(1, bins);
}

/**
 * @brief 设置频率范围
 * @param minFreq 最低频率(Hz)
 * @param maxFreq 最高频率(Hz)
 */
void MelSpectrogram7::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = qMax(0.0, minFreq);
    m_maxFreq = qMax(m_minFreq + 1.0, maxFreq);
}

/**
 * @brief 频率转Mel刻度
 */
static double hzToMel(double hz)
{
    return 2595.0 * qLn(1.0 + hz / 700.0) / qLn(10.0);
}

/**
 * @brief Mel刻度转频率
 */
static double melToHz(double mel)
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/**
 * @brief 就地基2 FFT
 */
static void radix2FFT(QVector<double>& re, QVector<double>& im)
{
    const int n = re.size();
    if (n <= 1) return;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wnRe = qCos(ang), wnIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double wRe = 1.0, wIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = wRe * re[i+j+len/2] - wIm * im[i+j+len/2];
                double tIm = wRe * im[i+j+len/2] + wIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double nw = wRe * wnRe - wIm * wnIm;
                wIm = wRe * wnIm + wIm * wnRe;
                wRe = nw;
            }
        }
    }
}

/**
 * @brief 构建Mel三角滤波器组
 * @param melBins Mel频带数
 * @param fftSize FFT大小
 * @param sampleRate 采样率
 * @param minFreq 最低频率
 * @param maxFreq 最高频率
 * @return 滤波器组矩阵(melBins × fftSize/2+1)
 */
static QVector<QVector<double>> buildMelFilterBank(int melBins, int fftSize,
    double sampleRate, double minFreq, double maxFreq)
{
    const int numFftBins = fftSize / 2 + 1;
    double melMin = hzToMel(minFreq);
    double melMax = hzToMel(maxFreq);

    /* 在Mel刻度上等间隔取melBins+2个点 */
    QVector<double> melPoints(melBins + 2);
    for (int i = 0; i < melBins + 2; ++i)
        melPoints[i] = melMin + (melMax - melMin) * i / (melBins + 1);

    /* 转回Hz并映射到FFT bin索引 */
    QVector<int> binPoints(melBins + 2);
    for (int i = 0; i < melBins + 2; ++i)
        binPoints[i] = static_cast<int>(qRound(melToHz(melPoints[i]) / sampleRate * fftSize));

    /* 构建三角滤波器 */
    QVector<QVector<double>> filterBank(melBins, QVector<double>(numFftBins, 0.0));
    for (int m = 0; m < melBins; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];
        if (center <= left || right <= center) continue;
        for (int k = left; k <= qMin(right, numFftBins - 1); ++k) {
            if (k <= center && center > left)
                filterBank[m][k] = static_cast<double>(k - left) / (center - left);
            else if (k > center && right > center)
                filterBank[m][k] = static_cast<double>(right - k) / (right - center);
        }
    }
    return filterBank;
}

/**
 * @brief 对输入音频计算Mel频谱图
 *
 * 分帧加窗 → FFT → 功率谱 → Mel滤波器组 → 对数能量。
 *
 * @param audio 输入音频信号
 * @return 每帧的Mel能量向量
 */
QVector<QVector<double>> MelSpectrogram7::compute(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result;
    const int N = audio.size();
    if (N == 0) {
        emit computationCompleted(0, 0);
        return result;
    }

    /* 帧参数 */
    int frameSize = m_fftSize;
    int hopSize = frameSize / 4;
    int numFrames = qMax(1, (N - frameSize) / hopSize + 1);

    /* 构建Mel滤波器组 */
    auto filterBank = buildMelFilterBank(m_melBins, m_fftSize,
                                          m_sampleRate, m_minFreq, m_maxFreq);
    int numFftBins = m_fftSize / 2 + 1;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;

        /* 加窗FFT */
        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);
        for (int i = 0; i < frameSize && start + i < N; ++i) {
            double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * i / frameSize));
            re[i] = audio[start + i] * hann;
        }
        radix2FFT(re, im);

        /* 计算功率谱 */
        QVector<double> power(numFftBins);
        for (int i = 0; i < numFftBins; ++i)
            power[i] = (re[i] * re[i] + im[i] * im[i]) / m_fftSize;

        /* 应用Mel滤波器组并取对数 */
        QVector<double> melFrame(m_melBins, 0.0);
        for (int m = 0; m < m_melBins; ++m) {
            double sum = 0.0;
            for (int i = 0; i < numFftBins; ++i)
                sum += power[i] * filterBank[m][i];
            melFrame[m] = qLn(sum + 1e-10);
        }
        result.append(melFrame);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(result.size(), m_melBins);
    return result;
}
