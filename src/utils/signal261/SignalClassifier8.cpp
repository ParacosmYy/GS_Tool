/**
 * @file SignalClassifier8.cpp
 * @brief SignalClassifier8 实现
 *
 * 实现信号分类器：时频图像表征与归一化互相关模板匹配。
 */

#include "utils/signal261/SignalClassifier8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalClassifier8::SignalClassifier8(QObject *parent)
    : QObject(parent) {}
SignalClassifier8::~SignalClassifier8() = default;

/* ---- Configuration ---- */

void SignalClassifier8::setWindowSize(int size) { m_windowSize = qMax(8, size); m_freqBins = m_windowSize / 2 + 1; }
void SignalClassifier8::setHopSize(int hop) { m_hopSize = qMax(1, hop); }

/* ---- Hann window ---- */

void SignalClassifier8::applyHannWindow(QVector<double>& frame) const
{
    int n = frame.size();
    for (int i = 0; i < n; ++i)
        frame[i] *= 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
}

/* ---- Magnitude spectrum ---- */

QVector<double> SignalClassifier8::magnitudeSpectrum(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> real = frame, imag(n, 0.0);

    // Simple DFT (for correctness; performance via STFT frame size)
    QVector<double> mag(m_freqBins, 0.0);
    for (int k = 0; k < m_freqBins && k <= n / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += real[i] * qCos(angle);
            im -= real[i] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- Compute TF image ---- */

QVector<QVector<double>> SignalClassifier8::computeTFImage(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < m_windowSize) {
        // Pad if too short
        QVector<double> padded = signal;
        padded.resize(m_windowSize, 0.0);
        return computeTFImage(padded);
    }

    int numFrames = (n - m_windowSize) / m_hopSize + 1;
    QVector<QVector<double>> tfImage(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        QVector<double> frame(m_windowSize);
        for (int i = 0; i < m_windowSize; ++i)
            frame[i] = signal[f * m_hopSize + i];
        applyHannWindow(frame);
        tfImage[f] = magnitudeSpectrum(frame);
    }
    return tfImage;
}

/* ---- Resize image via bilinear interpolation ---- */

QVector<QVector<double>> SignalClassifier8::resizeImage(
    const QVector<QVector<double>>& img, int targetTime, int targetFreq) const
{
    int srcTime = img.size();
    if (srcTime == 0) return QVector<QVector<double>>(targetTime, QVector<double>(targetFreq, 0.0));
    int srcFreq = img[0].size();

    QVector<QVector<double>> result(targetTime, QVector<double>(targetFreq, 0.0));
    for (int t = 0; t < targetTime; ++t) {
        double srcT = static_cast<double>(t) * (srcTime - 1) / qMax(targetTime - 1, 1);
        int t0 = qFloor(srcT);
        int t1 = qMin(t0 + 1, srcTime - 1);
        double wt = srcT - t0;

        for (int f = 0; f < targetFreq; ++f) {
            double srcF = static_cast<double>(f) * (srcFreq - 1) / qMax(targetFreq - 1, 1);
            int f0 = qFloor(srcF);
            int f1 = qMin(f0 + 1, srcFreq - 1);
            double wf = srcF - f0;

            result[t][f] = (1 - wt) * (1 - wf) * img[t0][f0]
                          + (1 - wt) * wf * img[t0][f1]
                          + wt * (1 - wf) * img[t1][f0]
                          + wt * wf * img[t1][f1];
        }
    }
    return result;
}

/* ---- Normalized cross-correlation ---- */

double SignalClassifier8::normalizedCrossCorrelation(
    const QVector<QVector<double>>& a, const QVector<QVector<double>>& b) const
{
    int rows = qMin(a.size(), b.size());
    if (rows == 0) return 0.0;
    int cols = qMin(a[0].size(), b[0].size());

    // Compute means
    double meanA = 0.0, meanB = 0.0;
    int count = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (c < a[r].size() && c < b[r].size()) {
                meanA += a[r][c];
                meanB += b[r][c];
                count++;
            }
        }
    }
    if (count == 0) return 0.0;
    meanA /= count;
    meanB /= count;

    double num = 0.0, denA = 0.0, denB = 0.0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (c < a[r].size() && c < b[r].size()) {
                double da = a[r][c] - meanA;
                double db = b[r][c] - meanB;
                num += da * db;
                denA += da * da;
                denB += db * db;
            }
        }
    }

    double denom = qSqrt(denA * denB);
    return (denom > 1e-12) ? num / denom : 0.0;
}

/* ---- Add template ---- */

void SignalClassifier8::addTemplate(int label, const QString& name, const QVector<double>& signal)
{
    Template tmpl;
    tmpl.label = label;
    tmpl.name = name;
    tmpl.tfImage = computeTFImage(signal);
    tmpl.timeBins = tmpl.tfImage.size();
    tmpl.freqBins = (tmpl.timeBins > 0) ? tmpl.tfImage[0].size() : 0;
    m_templates.append(tmpl);
    m_stats.numTemplates = m_templates.size();
}

/* ---- Classify ---- */

SignalClassifier8::Result SignalClassifier8::classify(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    if (m_templates.isEmpty()) {
        result.label = -1;
        result.confidence = 0.0;
        return result;
    }

    QVector<QVector<double>> queryImage = computeTFImage(signal);
    result.scores.resize(m_templates.size());

    for (int i = 0; i < m_templates.size(); ++i) {
        // Resize query to match template dimensions
        int tTime = m_templates[i].timeBins;
        int tFreq = m_templates[i].freqBins;
        QVector<QVector<double>> resized = resizeImage(queryImage, tTime, tFreq);
        result.scores[i] = normalizedCrossCorrelation(resized, m_templates[i].tfImage);
    }

    // Find best match
    int bestIdx = 0;
    for (int i = 1; i < result.scores.size(); ++i)
        if (result.scores[i] > result.scores[bestIdx]) bestIdx = i;

    result.label = m_templates[bestIdx].label;
    // Confidence: how much better is the best vs second-best
    double bestScore = result.scores[bestIdx];
    double secondBest = -1.0;
    for (int i = 0; i < result.scores.size(); ++i)
        if (i != bestIdx && result.scores[i] > secondBest) secondBest = result.scores[i];
    result.confidence = qBound(0.0, (bestScore - secondBest + 1.0) / 2.0, 1.0);

    double elapsed = timer.elapsed();
    m_stats.numClassifications++;
    m_confidenceSum += result.confidence;
    m_stats.avgConfidence = m_confidenceSum / m_stats.numClassifications;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit classificationCompleted(result.label, result.confidence, elapsed);
    return result;
}

/* ---- Accessors ---- */

QVector<SignalClassifier8::Template> SignalClassifier8::templates() const { return m_templates; }

/* ---- Reset ---- */

void SignalClassifier8::resetStatistics()
{
    m_templates.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_confidenceSum = 0.0;
}
