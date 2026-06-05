/**
 * @file SpectralSubtract.cpp
 * @brief 谱减法降噪实现 — 噪声估计 + 过减 + 频谱下限 + Wiener后处理
 */

#include "utils/signal19/SpectralSubtract.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>

/* ──────────────────── 构造/析构 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
SpectralSubtract::SpectralSubtract(QObject* parent)
    : QObject(parent)
    , m_noiseEstimated(false)
    , m_frameCount(0)
{
    m_noiseMagnitude.resize(m_params.fftSize / 2 + 1, 0.0);
    m_noisePower.resize(m_params.fftSize / 2 + 1, 0.0);
    m_prevMagnitude.resize(m_params.fftSize / 2 + 1, 0.0);
    m_minTrackBuf.resize(m_params.fftSize / 2 + 1, 1e10);
    m_mmseNoise.resize(m_params.fftSize / 2 + 1, 0.0);
}

/** @brief 析构函数 */
SpectralSubtract::~SpectralSubtract() = default;

/* ──────────────────── 配置 ──────────────────── */

/** @brief 设置降噪参数 @param params 参数 */
void SpectralSubtract::setParameters(const Parameters& params)
{
    m_params = params;
    int halfN = params.fftSize / 2 + 1;
    m_noiseMagnitude.resize(halfN, 0.0);
    m_noisePower.resize(halfN, 0.0);
    m_prevMagnitude.resize(halfN, 0.0);
    m_minTrackBuf.resize(halfN, 1e10);
    m_mmseNoise.resize(halfN, 0.0);
    m_noiseEstimated = false;
    m_frameCount = 0;
}

/** @brief 获取当前参数 @return 参数 */
SpectralSubtract::Parameters SpectralSubtract::parameters() const
{
    return m_params;
}

/* ──────────────────── 降噪处理(自动噪声估计) ──────────────────── */

/** @brief 处理完整信号(自动噪声估计) @param signal 输入含噪信号 @return 降噪后信号 */
QVector<double> SpectralSubtract::process(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_params.fftSize;
    int hop = m_params.hopSize;
    int halfN = N / 2 + 1;
    int n = signal.size();

    if (n < N) return signal;

    int numFrames = (n - N) / hop + 1;
    QVector<double> output(n, 0.0);
    QVector<double> windowSum(n, 0.0);

    /* Hann窗 */
    QVector<double> win(N);
    for (int i = 0; i < N; ++i) {
        win[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / static_cast<double>(N)));
    }

    m_frameCount = 0;
    double totalSnr = 0.0;
    double totalNoisePower = 0.0;

    for (int frame = 0; frame < numFrames; ++frame) {
        int pos = frame * hop;

        /* 提取帧 */
        std::vector<double> re(N, 0.0), im(N, 0.0);
        for (int i = 0; i < N; ++i) {
            re[i] = signal[pos + i] * win[i];
        }

        /* FFT */
        computeFFT(re, im);

        /* 幅度谱 */
        QVector<double> magnitude(halfN);
        QVector<double> power(halfN);
        for (int i = 0; i < halfN; ++i) {
            magnitude[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
            power[i] = magnitude[i] * magnitude[i];
        }

        /* 噪声估计 */
        if (!m_noiseEstimated) {
            if (m_params.noiseEstim == FirstFrames) {
                /* 首几帧平均 */
                for (int i = 0; i < halfN; ++i) {
                    m_noisePower[i] += power[i];
                    m_noiseMagnitude[i] += magnitude[i];
                }
                if (frame + 1 >= m_params.noiseFrames) {
                    for (int i = 0; i < halfN; ++i) {
                        m_noisePower[i] /= static_cast<double>(m_params.noiseFrames);
                        m_noiseMagnitude[i] /= static_cast<double>(m_params.noiseFrames);
                    }
                    m_noiseEstimated = true;
                    emit noiseEstimateUpdated(
                        std::accumulate(m_noisePower.begin(),
                                        m_noisePower.end(), 0.0) / halfN);
                }
            } else if (m_params.noiseEstim == MinTrack) {
                updateMinTrack(magnitude);
                if (frame + 1 >= m_params.noiseFrames) {
                    m_noiseEstimated = true;
                }
            } else if (m_params.noiseEstim == MMSE) {
                updateMMSE(magnitude);
                if (frame + 1 >= m_params.noiseFrames) {
                    m_noiseEstimated = true;
                }
            }
        } else {
            /* 在线噪声更新 */
            if (m_params.noiseEstim == MinTrack) {
                updateMinTrack(magnitude);
            } else if (m_params.noiseEstim == MMSE) {
                updateMMSE(magnitude);
            }
        }

        /* 谱减法 */
        QVector<double> cleanPower(halfN);
        for (int i = 0; i < halfN; ++i) {
            /* 过减 */
            double alpha = m_params.overSubtraction;
            double beta = m_params.spectralFloor;
            double subtracted = power[i] - alpha * m_noisePower[i];

            /* 频谱下限 */
            double floor = beta * power[i];
            cleanPower[i] = qMax(subtracted, floor);

            /* 平滑 */
            if (m_params.enableSmoothing && frame > 0) {
                cleanPower[i] = m_params.smoothCoeff * m_prevMagnitude[i]
                              + (1.0 - m_params.smoothCoeff) * cleanPower[i];
            }
        }
        m_prevMagnitude = cleanPower;

        /* Wiener后处理 */
        if (m_params.enableWiener) {
            auto wg = wienerGain(cleanPower, m_noisePower);
            for (int i = 0; i < halfN; ++i) {
                cleanPower[i] *= wg[i] * m_params.wienerGain;
            }
        }

        /* 重建频谱 */
        for (int i = 0; i < halfN; ++i) {
            double magClean = qSqrt(qMax(cleanPower[i], 0.0));
            double phase = qAtan2(im[i], re[i]);
            re[i] = magClean * qCos(phase);
            im[i] = magClean * qSin(phase);
        }
        /* 对称扩展 */
        for (int i = halfN; i < N; ++i) {
            re[i] = re[N - i];
            im[i] = -im[N - i];
        }

        /* IFFT */
        computeIFFT(re, im);

        /* 重叠相加 */
        for (int i = 0; i < N && (pos + i) < n; ++i) {
            output[pos + i] += re[i] * win[i];
            windowSum[pos + i] += win[i] * win[i];
        }

        /* 计算帧SNR */
        double sigPow = std::accumulate(cleanPower.begin(), cleanPower.end(), 0.0);
        double noiPow = std::accumulate(m_noisePower.begin(), m_noisePower.end(), 0.0);
        double snr = noiPow > 0 ? 10.0 * qLn(sigPow / noiPow + 1e-30) / qLn(10.0) : 0.0;
        totalSnr += snr;
        totalNoisePower += noiPow;

        m_frameCount++;
        emit frameProcessed(frame, snr);
    }

    /* 归一化重叠相加 */
    for (int i = 0; i < n; ++i) {
        if (windowSum[i] > 1e-10) {
            output[i] /= windowSum[i];
        }
    }

    /* 统计 */
    m_stats.totalFramesProcessed += static_cast<quint64>(numFrames);
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    m_stats.avgSnr = totalSnr / static_cast<double>(qMax(numFrames, 1));
    m_stats.avgNoisePower = totalNoisePower / static_cast<double>(qMax(numFrames, 1));

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalFramesProcessed, 1ULL));

    emit processingCompleted(numFrames, m_stats.avgSnr);
    return output;
}

/* ──────────────────── 降噪处理(手动噪声参考) ──────────────────── */

/** @brief 处理完整信号(手动噪声参考) @param signal 含噪信号 @param noiseRef 噪声参考 @return 降噪后信号 */
QVector<double> SpectralSubtract::processWithNoiseRef(
    const QVector<double>& signal,
    const QVector<double>& noiseRef)
{
    /* 先估计噪声频谱 */
    m_noiseMagnitude = estimateNoise(noiseRef);
    int halfN = m_params.fftSize / 2 + 1;
    m_noisePower.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        m_noisePower[i] = m_noiseMagnitude[i] * m_noiseMagnitude[i];
    }
    m_noiseEstimated = true;

    emit noiseEstimateUpdated(
        std::accumulate(m_noisePower.begin(), m_noisePower.end(), 0.0) / halfN);

    return process(signal);
}

/* ──────────────────── 帧处理 ──────────────────── */

/** @brief 处理单帧 @param frame 输入帧 @param isFirst 是否首帧 @return (降噪后帧, 分析数据) */
QPair<QVector<double>, SpectralSubtract::FrameAnalysis> SpectralSubtract::processFrame(
    const QVector<double>& frame, bool isFirst)
{
    FrameAnalysis analysis;
    int N = m_params.fftSize;
    int halfN = N / 2 + 1;

    if (frame.size() < N) {
        return {frame, analysis};
    }

    /* 加窗 + FFT */
    QVector<double> windowed = applyWindow(frame);
    std::vector<double> re(N), im(N, 0.0);
    for (int i = 0; i < N; ++i) re[i] = windowed[i];
    computeFFT(re, im);

    QVector<double> magnitude(halfN), power(halfN);
    for (int i = 0; i < halfN; ++i) {
        magnitude[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
        power[i] = magnitude[i] * magnitude[i];
    }

    /* 首帧噪声估计 */
    if (isFirst || !m_noiseEstimated) {
        m_noiseMagnitude = magnitude;
        for (int i = 0; i < halfN; ++i) {
            m_noisePower[i] = power[i];
        }
        m_noiseEstimated = true;
    }

    /* 谱减 */
    QVector<double> cleanPower(halfN);
    for (int i = 0; i < halfN; ++i) {
        double sub = power[i] - m_params.overSubtraction * m_noisePower[i];
        cleanPower[i] = qMax(sub, m_params.spectralFloor * power[i]);
    }

    /* Wiener */
    if (m_params.enableWiener) {
        auto wg = wienerGain(cleanPower, m_noisePower);
        for (int i = 0; i < halfN; ++i) {
            cleanPower[i] *= wg[i];
        }
    }

    /* IFFT重建 */
    for (int i = 0; i < halfN; ++i) {
        double magClean = qSqrt(qMax(cleanPower[i], 0.0));
        double phase = qAtan2(im[i], re[i]);
        re[i] = magClean * qCos(phase);
        im[i] = magClean * qSin(phase);
    }
    for (int i = halfN; i < N; ++i) {
        re[i] = re[N - i];
        im[i] = -im[N - i];
    }
    computeIFFT(re, im);

    QVector<double> result(N);
    for (int i = 0; i < N; ++i) result[i] = re[i];

    /* 分析数据 */
    analysis.signalPower = std::accumulate(power.begin(), power.end(), 0.0) / halfN;
    analysis.noisePower = std::accumulate(m_noisePower.begin(), m_noisePower.end(), 0.0) / halfN;
    analysis.snr = analysis.noisePower > 0
        ? 10.0 * qLn(analysis.signalPower / analysis.noisePower + 1e-30) / qLn(10.0) : 0.0;
    analysis.reductionDb = analysis.signalPower > 0
        ? 10.0 * qLn(analysis.signalPower / (std::accumulate(cleanPower.begin(),
                   cleanPower.end(), 0.0) / halfN + 1e-30)) / qLn(10.0) : 0.0;

    return {result, analysis};
}

/* ──────────────────── 噪声估计 ──────────────────── */

/** @brief 手动设置噪声频谱 @param magnitude 噪声幅度谱 */
void SpectralSubtract::setNoiseSpectrum(const QVector<double>& magnitude)
{
    m_noiseMagnitude = magnitude;
    int halfN = magnitude.size();
    m_noisePower.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        m_noisePower[i] = magnitude[i] * magnitude[i];
    }
    m_noiseEstimated = true;
}

/** @brief 从噪声片段估计噪声频谱 @param noise 噪声参考信号 @return 噪声幅度谱 */
QVector<double> SpectralSubtract::estimateNoise(const QVector<double>& noise) const
{
    int N = m_params.fftSize;
    int halfN = N / 2 + 1;
    int nFrames = qMax(1, (noise.size() - N) / m_params.hopSize + 1);
    QVector<double> avgMag(halfN, 0.0);

    for (int f = 0; f < nFrames; ++f) {
        int pos = f * m_params.hopSize;
        QVector<double> windowed = applyWindow(noise.mid(pos, N));

        std::vector<double> re(N, 0.0), im(N, 0.0);
        for (int i = 0; i < qMin(N, windowed.size()); ++i) re[i] = windowed[i];
        computeFFT(re, im);

        for (int i = 0; i < halfN; ++i) {
            avgMag[i] += qSqrt(re[i] * re[i] + im[i] * im[i]);
        }
    }

    for (int i = 0; i < halfN; ++i) {
        avgMag[i] /= static_cast<double>(nFrames);
    }

    return avgMag;
}

/** @brief 获取当前噪声估计 @return 噪声幅度谱 */
QVector<double> SpectralSubtract::noiseSpectrum() const
{
    return m_noiseMagnitude;
}

/* ──────────────────── 私有方法 ──────────────────── */

/** @brief FFT(Cooley-Tukey) @param re 实部 @param im 虚部 */
void SpectralSubtract::computeFFT(std::vector<double>& re,
                                  std::vector<double>& im) const
{
    int n = static_cast<int>(re.size());
    /* 位反转 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    /* 蝶形 */
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / static_cast<double>(len);
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = cRe * re[v] - cIm * im[v];
                double tIm = cRe * im[v] + cIm * re[v];
                re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                re[u] += tRe; im[u] += tIm;
                double nRe = cRe * wRe - cIm * wIm;
                cIm = cRe * wIm + cIm * wRe; cRe = nRe;
            }
        }
    }
}

/** @brief IFFT @param re 实部 @param im 虚部 */
void SpectralSubtract::computeIFFT(std::vector<double>& re,
                                   std::vector<double>& im) const
{
    int n = static_cast<int>(re.size());
    /* 共轭 */
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    computeFFT(re, im);
    for (int i = 0; i < n; ++i) {
        re[i] /= static_cast<double>(n);
        im[i] = -im[i] / static_cast<double>(n);
    }
}

/** @brief 应用Hann窗 @param frame 输入帧 @return 加窗后帧 */
QVector<double> SpectralSubtract::applyWindow(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / static_cast<double>(n)));
        windowed[i] = frame[i] * w;
    }
    return windowed;
}

/** @brief Wiener滤波增益 @param signalPow 信号功率谱 @param noisePow 噪声功率谱 @return Wiener增益 */
QVector<double> SpectralSubtract::wienerGain(const QVector<double>& signalPow,
                                             const QVector<double>& noisePow) const
{
    int n = signalPow.size();
    QVector<double> gain(n);
    for (int i = 0; i < n; ++i) {
        double snr = (noisePow[i] > 1e-30) ? signalPow[i] / noisePow[i] : 1.0;
        /* Wiener增益 = SNR / (1 + SNR) */
        gain[i] = qMax(snr / (1.0 + snr), 0.0);
    }
    return gain;
}

/** @brief 最小值跟踪噪声估计更新 @param magnitude 当前帧幅度谱 */
void SpectralSubtract::updateMinTrack(const QVector<double>& magnitude)
{
    double alpha = m_params.minTrackAlpha;
    int halfN = magnitude.size();

    for (int i = 0; i < halfN; ++i) {
        /* 指数遗忘 */
        m_minTrackBuf[i] = qMin(m_minTrackBuf[i] / alpha, magnitude[i]);
        m_noisePower[i] = m_minTrackBuf[i] * m_minTrackBuf[i] * 1.5;
        m_noiseMagnitude[i] = m_minTrackBuf[i] * qSqrt(1.5);
    }
}

/** @brief MMSE噪声估计更新 @param magnitude 当前帧幅度谱 */
void SpectralSubtract::updateMMSE(const QVector<double>& magnitude)
{
    double alpha = m_params.minTrackAlpha;
    double p = m_params.minTrackProb;
    int halfN = magnitude.size();

    for (int i = 0; i < halfN; ++i) {
        /* MMSE最优估计: 条件期望 */
        double priorNoise = m_mmseNoise[i];
        double currentMag = magnitude[i];

        /* 语音存在概率更新 */
        double snrPost = currentMag * currentMag /
                        (priorNoise * priorNoise + 1e-30);
        double speechProb = 1.0 / (1.0 + (1.0 - p) / p * qExp(-snrPost));

        /* 噪声更新 */
        double noiseEst = speechProb * priorNoise
                        + (1.0 - speechProb) * currentMag;
        m_mmseNoise[i] = alpha * priorNoise + (1.0 - alpha) * noiseEst;
        m_noisePower[i] = m_mmseNoise[i] * m_mmseNoise[i];
        m_noiseMagnitude[i] = m_mmseNoise[i];
    }
}

/** @brief 重叠相加 @param frame 当前帧 @param output 输出缓冲区 @param pos 当前位置 */
void SpectralSubtract::overlapAdd(const QVector<double>& frame,
                                  QVector<double>& output, int pos) const
{
    int N = m_params.fftSize;
    QVector<double> win(N);
    for (int i = 0; i < N; ++i) {
        win[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / static_cast<double>(N)));
    }

    for (int i = 0; i < N && (pos + i) < output.size(); ++i) {
        output[pos + i] += frame[i] * win[i];
    }
}

/* ──────────────────── 统计 ──────────────────── */

/** @brief 获取统计 @return 统计 */
SpectralSubtract::Stats SpectralSubtract::stats() const { return m_stats; }

/** @brief 重置统计 */
void SpectralSubtract::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
