/**
 * @file ZoomFFT.cpp
 * @brief Zoom FFT实现 — 频带选择式高分辨率窄带频谱分析
 */

#include "utils/fft18/ZoomFFT.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>

/* ──────────────────── 构造/析构 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
ZoomFFT::ZoomFFT(QObject* parent)
    : QObject(parent)
{
    /* 预计算默认滤波器 */
    designDefaultFilter();
}

/** @brief 析构函数 */
ZoomFFT::~ZoomFFT() = default;

/** @brief 设计默认滤波器 */
void ZoomFFT::designDefaultFilter()
{
    double cutoff = m_params.bandwidth / 2.0;
    m_filterCoeffs = designLowpass(cutoff, m_params.sampleRate, m_params.filterOrder);
}

/* ──────────────────── 配置 ──────────────────── */

/** @brief 设置分析参数 @param params 参数 */
void ZoomFFT::setParameters(const Parameters& params)
{
    m_params = params;
    designDefaultFilter();
}

/** @brief 获取当前参数 @return 参数 */
ZoomFFT::Parameters ZoomFFT::parameters() const
{
    return m_params;
}

/* ──────────────────── 频谱分析(实信号) ──────────────────── */

/** @brief Zoom FFT分析(实信号) @param samples 输入采样 @return 频谱结果 */
ZoomFFT::SpectrumResult ZoomFFT::analyze(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;
    int n = samples.size();
    if (n < m_params.fftSize) {
        return result;
    }

    emit progressChanged(tr("频移"), 10);

    /* 步骤1: 频移(复混频到基带) */
    auto [iBase, qBase] = mixDown(samples, m_params.centerFreq);

    emit progressChanged(tr("低通滤波"), 30);

    /* 步骤2: 低通滤波 */
    iBase = applyFIR(iBase, m_filterCoeffs);
    qBase = applyFIR(qBase, m_filterCoeffs);

    emit progressChanged(tr("抽取"), 50);

    /* 步骤3: 抽取 */
    int decFactor = m_params.decimationFactor;
    iBase = decimate(iBase, decFactor);
    qBase = decimate(qBase, decFactor);

    emit progressChanged(tr("FFT"), 70);

    /* 步骤4: 基带FFT */
    int fftN = qMin(m_params.fftSize, iBase.size());
    if (fftN < 4) return result;

    /* 零填充到fftSize */
    std::vector<double> re(fftN, 0.0), im(fftN, 0.0);
    for (int i = 0; i < qMin(fftN, iBase.size()); ++i) {
        re[i] = iBase[i];
        im[i] = qBase[i];
    }

    /* 加Hann窗 */
    for (int i = 0; i < fftN; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / static_cast<double>(fftN)));
        re[i] *= w;
        im[i] *= w;
    }

    computeFFT(re, im);
    m_stats.totalFFTsComputed++;

    emit progressChanged(tr("计算频谱"), 90);

    /* 步骤5: 计算幅度和相位 */
    int halfN = fftN / 2;
    double newSampleRate = m_params.sampleRate / static_cast<double>(decFactor);
    double freqResolution = newSampleRate / static_cast<double>(fftN);

    result.frequencies.reserve(halfN);
    result.magnitudes.reserve(halfN);
    result.phases.reserve(halfN);

    for (int i = 0; i < halfN; ++i) {
        double freq = m_params.centerFreq - m_params.bandwidth / 2.0
                    + static_cast<double>(i) * freqResolution;
        result.frequencies.append(freq);

        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
        double magDb = 20.0 * qLn(mag + 1e-30) / qLn(10.0);
        result.magnitudes.append(magDb);

        double phase = qAtan2(im[i], re[i]);
        result.phases.append(phase);
    }

    result.resolutionHz = freqResolution;
    result.effectiveBandwidth = newSampleRate / 2.0;
    result.actualDecimation = decFactor;
    result.success = true;

    /* 统计 */
    m_stats.totalAnalyses++;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalAnalyses, 1ULL));

    emit analysisCompleted(result.resolutionHz, result.effectiveBandwidth);
    emit progressChanged(tr("完成"), 100);
    return result;
}

/* ──────────────────── 频谱分析(复信号) ──────────────────── */

/** @brief Zoom FFT分析(复信号) @param iSamples 同相分量 @param qSamples 正交分量 @return 频谱结果 */
ZoomFFT::SpectrumResult ZoomFFT::analyzeComplex(const QVector<double>& iSamples,
                                                  const QVector<double>& qSamples)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;
    int n = qMin(iSamples.size(), qSamples.size());
    if (n < m_params.fftSize) return result;

    /* 复信号已经分离为I/Q，直接滤波+抽取+FFT */
    QVector<double> iFilt = applyFIR(iSamples, m_filterCoeffs);
    QVector<double> qFilt = applyFIR(qSamples, m_filterCoeffs);

    int decFactor = m_params.decimationFactor;
    iFilt = decimate(iFilt, decFactor);
    qFilt = decimate(qFilt, decFactor);

    int fftN = qMin(m_params.fftSize, iFilt.size());
    if (fftN < 4) return result;

    std::vector<double> re(fftN, 0.0), im(fftN, 0.0);
    for (int i = 0; i < fftN; ++i) {
        re[i] = iFilt[i];
        im[i] = qFilt[i];
    }

    computeFFT(re, im);
    m_stats.totalFFTsComputed++;

    int halfN = fftN / 2;
    double newSampleRate = m_params.sampleRate / static_cast<double>(decFactor);
    double freqResolution = newSampleRate / static_cast<double>(fftN);

    result.frequencies.reserve(halfN);
    result.magnitudes.reserve(halfN);
    result.phases.reserve(halfN);

    for (int i = 0; i < halfN; ++i) {
        double freq = m_params.centerFreq - m_params.bandwidth / 2.0
                    + static_cast<double>(i) * freqResolution;
        result.frequencies.append(freq);
        double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
        result.magnitudes.append(20.0 * qLn(mag + 1e-30) / qLn(10.0));
        result.phases.append(qAtan2(im[i], re[i]));
    }

    result.resolutionHz = freqResolution;
    result.effectiveBandwidth = newSampleRate / 2.0;
    result.actualDecimation = decFactor;
    result.success = true;

    m_stats.totalAnalyses++;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalAnalyses, 1ULL));

    emit analysisCompleted(result.resolutionHz, result.effectiveBandwidth);
    return result;
}

/* ──────────────────── 频移 ──────────────────── */

/** @brief 仅执行频移+低通 @param samples 输入采样 @return 滤波后基带信号 */
QVector<double> ZoomFFT::frequencyShift(const QVector<double>& samples) const
{
    auto [iBase, qBase] = mixDown(samples, m_params.centerFreq);
    iBase = applyFIR(iBase, m_filterCoeffs);
    return iBase;
}

/* ──────────────────── 滤波器设计 ──────────────────── */

/** @brief 设计Kaiser窗低通滤波器 @param cutoff 截止频率 @param sampleRate 采样率 @param order 阶数 @return 滤波器系数 */
QVector<double> ZoomFFT::designLowpass(double cutoff, double sampleRate, int order) const
{
    int N = order + 1;
    QVector<double> coeffs(N);

    double beta = kaiserBeta(m_params.filterAttenuation);
    double fc = cutoff / sampleRate;

    for (int n = 0; n < N; ++n) {
        double w = kaiserWindow(n, N, beta);
        double ideal = (n == N / 2) ? 2.0 * fc
            : qSin(2.0 * M_PI * fc * (n - N / 2.0))
            / (M_PI * (n - N / 2.0));
        coeffs[n] = ideal * w;
    }

    /* 归一化使通带增益为1 */
    double sum = 0.0;
    for (double c : coeffs) sum += c;
    for (double& c : coeffs) c /= sum;

    return coeffs;
}

/* ──────────────────── 辅助 ──────────────────── */

/** @brief 计算有效频率分辨率 @return 分辨率(Hz) */
double ZoomFFT::effectiveResolution() const
{
    double newSr = m_params.sampleRate / static_cast<double>(m_params.decimationFactor);
    return newSr / static_cast<double>(m_params.fftSize);
}

/** @brief 计算所需最小输入长度 @return 最小采样数 */
int ZoomFFT::minimumInputLength() const
{
    return m_params.fftSize * m_params.decimationFactor + m_params.filterOrder;
}

/* ──────────────────── 私有方法 ──────────────────── */

/** @brief 频移(复数混频) @param samples 输入 @param freq 载波频率 @return (I,Q)基带信号 */
QPair<QVector<double>, QVector<double>> ZoomFFT::mixDown(
    const QVector<double>& samples, double freq) const
{
    int n = samples.size();
    QVector<double> iOut(n), qOut(n);
    double phaseInc = 2.0 * M_PI * freq / m_params.sampleRate;

    for (int i = 0; i < n; ++i) {
        double phase = phaseInc * i;
        iOut[i] = samples[i] * qCos(phase);
        qOut[i] = -samples[i] * qSin(phase); /* 负号=下变频 */
    }

    return {iOut, qOut};
}

/** @brief FIR低通滤波 @param signal 输入信号 @param coeffs 滤波器系数 @return 滤波后信号 */
QVector<double> ZoomFFT::applyFIR(const QVector<double>& signal,
                                  const QVector<double>& coeffs) const
{
    int n = signal.size();
    int order = coeffs.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < order; ++j) {
            int idx = i - j;
            if (idx >= 0) {
                sum += coeffs[j] * signal[idx];
            }
        }
        output[i] = sum;
    }

    return output;
}

/** @brief 抽取(降采样) @param signal 输入信号 @param factor 抽取因子 @return 抽取后信号 */
QVector<double> ZoomFFT::decimate(const QVector<double>& signal, int factor) const
{
    QVector<double> output;
    output.reserve(signal.size() / factor);

    for (int i = 0; i < signal.size(); i += factor) {
        output.append(signal[i]);
    }

    return output;
}

/** @brief Cooley-Tukey FFT @param re 实部 @param im 虚部 */
void ZoomFFT::computeFFT(std::vector<double>& re, std::vector<double>& im) const
{
    int n = static_cast<int>(re.size());

    /* 位反转 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / static_cast<double>(len);
        double wRe = qCos(ang);
        double wIm = qSin(ang);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

/** @brief Kaiser窗函数值 @param n 采样索引 @param N 总长度 @param beta 形状参数 @return 窗值 */
double ZoomFFT::kaiserWindow(int n, int N, double beta) const
{
    double alpha = static_cast<double>(N - 1) / 2.0;
    double x = (static_cast<double>(n) - alpha) / alpha;
    return besselI0(beta * qSqrt(1.0 - x * x)) / besselI0(beta);
}

/** @brief 计算Kaiser窗beta @param attenuation 阻带衰减(dB) @return beta */
double ZoomFFT::kaiserBeta(double attenuation) const
{
    if (attenuation > 50.0) return 0.1102 * (attenuation - 8.7);
    if (attenuation > 21.0) return 0.5842 * qPow(attenuation - 21.0, 0.4)
                             + 0.07886 * (attenuation - 21.0);
    return 0.0;
}

/** @brief 零阶修正Bessel函数I0 @param x 参数 @return I0(x) */
double ZoomFFT::besselI0(double x) const
{
    double sum = 1.0;
    double term = 1.0;
    double xSq4 = x * x / 4.0;

    for (int k = 1; k < 30; ++k) {
        term *= xSq4 / (static_cast<double>(k) * static_cast<double>(k));
        sum += term;
        if (term < 1e-15 * sum) break;
    }

    return sum;
}

/* ──────────────────── 统计 ──────────────────── */

/** @brief 获取统计 @return 统计 */
ZoomFFT::Stats ZoomFFT::stats() const { return m_stats; }

/** @brief 重置统计 */
void ZoomFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
