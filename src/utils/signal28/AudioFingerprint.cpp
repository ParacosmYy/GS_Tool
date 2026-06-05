/**
 * @file AudioFingerprint.cpp
 * @brief 音频指纹实现
 */

#include "utils/signal28/AudioFingerprint.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

AudioFingerprint::AudioFingerprint(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_fftSize(1024)
    , m_peakThreshold(10.0)
    , m_fanOut(15)
    , m_targetZoneWidth(100)
    , m_timeSum(0.0)
{
}

void AudioFingerprint::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

void AudioFingerprint::setFftSize(int size)
{
    m_fftSize = qMax(64, size);
    /* 确保是2的幂 */
    int p = 1;
    while (p < m_fftSize) p <<= 1;
    m_fftSize = p;
}

void AudioFingerprint::setPeakThreshold(double threshold)
{
    m_peakThreshold = qMax(0.0, threshold);
}

void AudioFingerprint::setFanOut(int fanOut)
{
    m_fanOut = qMax(1, fanOut);
}

void AudioFingerprint::setTargetZoneWidth(int frames)
{
    m_targetZoneWidth = qMax(1, frames);
}

QVector<QVector<double>> AudioFingerprint::computeSpectrogram(
    const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> spectrogram;
    if (audio.isEmpty()) return spectrogram;

    int hopSize = m_fftSize / 2;
    int numFrames = (audio.size() - m_fftSize) / hopSize + 1;
    if (numFrames <= 0) return spectrogram;

    spectrogram.reserve(numFrames);
    for (int frame = 0; frame < numFrames; ++frame) {
        QVector<double> windowed(m_fftSize, 0.0);
        int offset = frame * hopSize;
        for (int i = 0; i < m_fftSize && offset + i < audio.size(); ++i) {
            double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
            windowed[i] = audio[offset + i] * hann;
        }
        QVector<double> magnitude;
        fftFrame(windowed, magnitude);
        spectrogram.append(magnitude);
    }
    return spectrogram;
}

QList<AudioFingerprint::PeakPoint> AudioFingerprint::extractPeaks(
    const QVector<QVector<double>>& spectrogram)
{
    QList<PeakPoint> allPeaks;
    int numFrames = spectrogram.size();
    int numBins = (numFrames > 0) ? spectrogram[0].size() : 0;

    for (int t = 0; t < numFrames; ++t) {
        QList<PeakPoint> framePeaks = findLocalPeaks(spectrogram[t], t);
        for (const auto& peak : framePeaks) {
            if (peak.magnitude > m_peakThreshold) {
                allPeaks.append(peak);
            }
        }
    }

    /* 按幅度降序排序，保留显著峰值 */
    std::sort(allPeaks.begin(), allPeaks.end(),
        [](const PeakPoint& a, const PeakPoint& b) {
            return a.magnitude > b.magnitude;
        });

    return allPeaks;
}

QList<AudioFingerprint::FingerprintHash> AudioFingerprint::generateFingerprint(
    const QList<PeakPoint>& peaks, int sourceId)
{
    QElapsedTimer timer;
    timer.start();

    QList<FingerprintHash> fingerprints;
    int n = peaks.size();

    for (int i = 0; i < n; ++i) {
        const PeakPoint& anchor = peaks[i];
        /* 扇出: 锚点与目标区域内的点组合 */
        int count = 0;
        for (int j = i + 1; j < n && count < m_fanOut; ++j) {
            const PeakPoint& target = peaks[j];
            int dt = target.timeFrame - anchor.timeFrame;

            /* 目标区域过滤 */
            if (dt <= 0 || dt > m_targetZoneWidth) continue;

            FingerprintHash fp;
            fp.hash = combinatorialHash(anchor, target);
            fp.timeOffset = anchor.timeFrame;
            fp.sourceId = sourceId;
            fingerprints.append(fp);
            ++count;
        }
    }

    ++m_stats.totalFingerprintsGenerated;
    m_timeSum += timer.elapsed();
    if (m_stats.totalFingerprintsGenerated > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFingerprintsGenerated;

    emit fingerprintGenerated(fingerprints.size(), sourceId);
    return fingerprints;
}

void AudioFingerprint::registerFingerprint(const QList<FingerprintHash>& fingerprints)
{
    for (const auto& fp : fingerprints) {
        m_database[fp.hash].append(fp);
    }
}

AudioFingerprint::MatchResult AudioFingerprint::query(
    const QList<FingerprintHash>& fingerprints)
{
    QElapsedTimer timer;
    timer.start();

    MatchResult result;
    result.found = false;
    result.sourceId = -1;
    result.confidence = 0.0;
    result.matchedPairs = 0;
    result.timeOffsetSec = 0.0;

    if (fingerprints.isEmpty()) return result;

    /* 时间偏移直方图: hash -> (sourceId, timeOffsetDelta) */
    QHash<int, QHash<int, int>> histogram;
    int totalLookups = 0;

    for (const auto& fp : fingerprints) {
        auto it = m_database.find(fp.hash);
        if (it == m_database.end()) continue;

        for (const auto& dbFp : it.value()) {
            int dt = dbFp.timeOffset - fp.timeOffset;
            histogram[dbFp.sourceId][dt]++;
            ++totalLookups;
        }
    }

    /* 找最大匹配 */
    int bestSource = -1;
    int bestDelta = 0;
    int bestCount = 0;
    int totalHashes = fingerprints.size();

    for (auto srcIt = histogram.constBegin(); srcIt != histogram.constEnd(); ++srcIt) {
        for (auto dtIt = srcIt.value().constBegin(); dtIt != srcIt.value().constEnd(); ++dtIt) {
            if (dtIt.value() > bestCount) {
                bestCount = dtIt.value();
                bestSource = srcIt.key();
                bestDelta = dtIt.key();
            }
        }
    }

    if (bestCount > 0) {
        result.found = true;
        result.sourceId = bestSource;
        result.matchedPairs = bestCount;
        result.confidence = qMin(1.0, static_cast<double>(bestCount) /
            qMax(1, qMin(totalHashes, 100)));
        double hopSize = m_fftSize / 2;
        result.timeOffsetSec = bestDelta * hopSize / m_sampleRate;
        ++m_stats.totalHits;
    }

    ++m_stats.totalQueries;
    m_timeSum += timer.elapsed();
    if (m_stats.totalQueries > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    emit matchResult(result.found, result.confidence);
    return result;
}

AudioFingerprint::MatchResult AudioFingerprint::identify(const QVector<double>& audio)
{
    QVector<QVector<double>> spec = computeSpectrogram(audio);
    QList<PeakPoint> peaks = extractPeaks(spec);
    QList<FingerprintHash> fps = generateFingerprint(peaks, -1);
    return query(fps);
}

void AudioFingerprint::clearDatabase()
{
    m_database.clear();
}

void AudioFingerprint::fftFrame(const QVector<double>& frame,
    QVector<double>& magnitude) const
{
    int n = frame.size();
    QVector<double> real = frame;
    QVector<double> imag(n, 0.0);

    /* 基2 FFT */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nR;
            }
        }
    }

    /* 幅度谱(单侧) */
    int halfN = n / 2 + 1;
    magnitude.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        double db = 20.0 * qLn(qSqrt(real[i] * real[i] + imag[i] * imag[i])
            + 1e-10) / qLn(10.0);
        magnitude[i] = db;
    }
}

QList<AudioFingerprint::PeakPoint> AudioFingerprint::findLocalPeaks(
    const QVector<double>& magnitude, int timeFrame) const
{
    QList<PeakPoint> peaks;
    int n = magnitude.size();
    int neighborhood = 5;

    for (int i = neighborhood; i < n - neighborhood; ++i) {
        bool isPeak = true;
        for (int j = i - neighborhood; j <= i + neighborhood; ++j) {
            if (j != i && magnitude[j] >= magnitude[i]) {
                isPeak = false;
                break;
            }
        }
        if (isPeak) {
            PeakPoint p;
            p.timeFrame = timeFrame;
            p.freqBin = i;
            p.magnitude = magnitude[i];
            peaks.append(p);
        }
    }
    return peaks;
}

quint64 AudioFingerprint::combinatorialHash(const PeakPoint& anchor,
    const PeakPoint& target) const
{
    /* 组合: freq1 | freq2 | deltaTime */
    int dt = target.timeFrame - anchor.timeFrame;
    quint64 f1 = static_cast<quint64>(anchor.freqBin & 0x3FF);
    quint64 f2 = static_cast<quint64>(target.freqBin & 0x3FF);
    quint64 d = static_cast<quint64>(dt & 0xFFFF);
    return (f1 << 26) | (f2 << 16) | d;
}

void AudioFingerprint::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
