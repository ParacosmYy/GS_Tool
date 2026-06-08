/**
 * @file PitchDetector6.cpp
 * @brief PitchDetector6 实现
 *
 * 实现音高检测：倒频谱基频估计与多候选HMM音高跟踪。
 */

#include "utils/signal232/PitchDetector6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PitchDetector6::PitchDetector6(QObject *parent) : QObject(parent) {}
PitchDetector6::~PitchDetector6() = default;

/* ---- Configure ---- */

bool PitchDetector6::configure(int frameSize, int hopSize, double sampleRate,
                                 double f0Min, double f0Max)
{
    if (frameSize < 64 || hopSize < 1 || sampleRate <= 0) return false;
    if (f0Min >= f0Max) return false;

    m_frameSize = frameSize;
    m_hopSize = hopSize;
    m_sampleRate = sampleRate;
    m_f0Min = f0Min;
    m_f0Max = f0Max;

    m_lastCepstrum.resize(frameSize, 0.0);
    m_stats.frameSize = frameSize;
    m_stats.hopSize = hopSize;
    return true;
}

/* ---- Compute real cepstrum via FFT ---- */

QVector<double> PitchDetector6::computeCepstrum(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> cep(n, 0.0);

    // Simplified DFT-based cepstrum (not full FFT, but direct computation)
    // For efficiency, we compute the power spectrum and then IDFT of log magnitudes
    QVector<double> powerSpec(n / 2, 0.0);

    for (int k = 0; k < n / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += frame[i] * qCos(angle);
            im -= frame[i] * qSin(angle);
        }
        powerSpec[k] = re * re + im * im;
    }

    // Log power spectrum
    QVector<double> logSpec(n / 2);
    for (int k = 0; k < n / 2; ++k)
        logSpec[k] = qLn(qMax(1e-10, powerSpec[k]));

    // IDFT of log spectrum to get cepstrum
    for (int q = 0; q < n; ++q) {
        double val = 0.0;
        for (int k = 0; k < n / 2; ++k)
            val += logSpec[k] * qCos(2.0 * M_PI * k * q / n);
        cep[q] = val / (n / 2);
    }
    return cep;
}

/* ---- Find cepstral peak ---- */

double PitchDetector6::findCepstralPeak(const QVector<double>& cep,
                                          double& confidence) const
{
    // Search in quefrency range corresponding to F0
    int minQ = qMax(1, static_cast<int>(m_sampleRate / m_f0Max));
    int maxQ = qMin(cep.size() / 2, static_cast<int>(m_sampleRate / m_f0Min));

    double bestVal = -1e30;
    int bestQ = minQ;

    // Average magnitude in cepstral region for normalization
    double avgMag = 0.0;
    for (int q = minQ; q <= maxQ; ++q)
        avgMag += qAbs(cep[q]);
    avgMag /= (maxQ - minQ + 1);

    for (int q = minQ; q <= maxQ; ++q) {
        if (cep[q] > bestVal) {
            bestVal = cep[q];
            bestQ = q;
        }
    }

    confidence = (avgMag > 1e-10) ? qAbs(bestVal) / avgMag : 0.0;
    confidence = qMin(1.0, confidence / 3.0);  // Normalize

    return (bestQ > 0) ? m_sampleRate / bestQ : 0.0;
}

/* ---- Generate pitch candidates ---- */

QVector<PitchDetector6::HMMCandidate> PitchDetector6::generateCandidates(
    const QVector<double>& cep) const
{
    QVector<HMMCandidate> candidates;

    int minQ = qMax(1, static_cast<int>(m_sampleRate / m_f0Max));
    int maxQ = qMin(cep.size() / 2, static_cast<int>(m_sampleRate / m_f0Min));

    // Find top N peaks in cepstrum
    QVector<QPair<double, int>> peaks;
    for (int q = minQ; q <= maxQ; ++q) {
        if (q > 0 && q < cep.size() - 1 && cep[q] > cep[q - 1] && cep[q] > cep[q + 1])
            peaks.append(qMakePair(cep[q], q));
    }

    std::sort(peaks.begin(), peaks.end());
    std::reverse(peaks.begin(), peaks.end());

    double totalWeight = 0.0;
    for (int i = 0; i < qMin(m_numCandidates, peaks.size()); ++i) {
        HMMCandidate cand;
        cand.frequency = m_sampleRate / peaks[i].second;
        cand.probability = qMax(0.0, peaks[i].first);
        totalWeight += cand.probability;
        cand.prevCandidate = -1;
        candidates.append(cand);
    }

    // Normalize probabilities
    if (totalWeight > 0.0)
        for (auto& c : candidates)
            c.probability /= totalWeight;

    // Add unvoiced candidate
    HMMCandidate unvoiced;
    unvoiced.frequency = 0.0;
    unvoiced.probability = 0.3;  // Prior for unvoiced
    unvoiced.prevCandidate = -1;
    candidates.append(unvoiced);

    return candidates;
}

/* ---- Voicing probability ---- */

double PitchDetector6::voicingProbability(double energy, double peakProminence) const
{
    double energyProb = 1.0 - qExp(-energy / 0.01);
    return 0.5 * energyProb + 0.5 * qMin(1.0, peakProminence);
}

/* ---- HMM transition cost ---- */

double PitchDetector6::transitionCost(double f1, double f2) const
{
    if (f1 <= 0.0 || f2 <= 0.0) return 1.0;  // voicing transition cost

    // Cost in semitones
    double semitones = 12.0 * qAbs(qLn(f2 / f1)) / qLn(2.0);
    return qExp(-semitones * semitones / (2.0 * m_transitionStd * m_transitionStd));
}

/* ---- Viterbi backtrace ---- */

QVector<PitchDetector6::PitchFrame> PitchDetector6::viterbiBacktrace(
    const QVector<QVector<HMMCandidate>>& trellis) const
{
    int numFrames = trellis.size();
    if (numFrames == 0) return QVector<PitchFrame>();

    // Find best final candidate
    int bestIdx = 0;
    double bestProb = trellis.last()[0].probability;
    for (int j = 1; j < trellis.last().size(); ++j) {
        if (trellis.last()[j].probability > bestProb) {
            bestProb = trellis.last()[j].probability;
            bestIdx = j;
        }
    }

    // Backtrace
    QVector<int> path(numFrames);
    path[numFrames - 1] = bestIdx;
    for (int t = numFrames - 2; t >= 0; --t)
        path[t] = trellis[t + 1][path[t + 1]].prevCandidate;

    // Convert to PitchFrames
    QVector<PitchFrame> result(numFrames);
    for (int t = 0; t < numFrames; ++t) {
        const HMMCandidate& c = trellis[t][path[t]];
        result[t].frequency = c.frequency;
        result[t].confidence = c.probability;
        result[t].voiced = (c.frequency > 0.0);
        result[t].energy = 0.0;
    }
    return result;
}

/* ---- Frame energy ---- */

double PitchDetector6::frameEnergy(const QVector<double>& frame) const
{
    double sum = 0.0;
    for (double s : frame) sum += s * s;
    return qSqrt(sum / qMax(1, frame.size()));
}

/* ---- Detect single frame ---- */

PitchDetector6::PitchFrame PitchDetector6::detectFrame(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    PitchFrame result;

    m_lastCepstrum = computeCepstrum(frame);
    double confidence = 0.0;
    double f0 = findCepstralPeak(m_lastCepstrum, confidence);
    double energy = frameEnergy(frame);

    m_lastCandidates = generateCandidates(m_lastCepstrum);

    double voicing = voicingProbability(energy, confidence);
    result.frequency = (voicing > 0.5) ? f0 : 0.0;
    result.confidence = confidence;
    result.voiced = (result.frequency > 0.0);
    result.energy = energy;

    m_stats.numFrames++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit frameDetected(result.frequency, result.voiced, result.confidence, timer.elapsed());
    return result;
}

/* ---- Detect trajectory ---- */

QVector<PitchDetector6::PitchFrame> PitchDetector6::detectTrajectory(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int numFrames = qMax(1, (signal.size() - m_frameSize) / m_hopSize + 1);
    m_trellis.resize(numFrames);

    // Build HMM trellis
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        int end = qMin(start + m_frameSize, signal.size());

        QVector<double> frame(end - start);
        for (int i = start; i < end; ++i)
            frame[i - start] = signal[i];
        // Zero-pad if needed
        while (frame.size() < m_frameSize)
            frame.append(0.0);

        QVector<double> cep = computeCepstrum(frame);
        QVector<HMMCandidate> candidates = generateCandidates(cep);
        double energy = frameEnergy(frame);

        // Add observation probabilities
        double conf = 0.0;
        findCepstralPeak(cep, conf);
        double voicing = voicingProbability(energy, conf);

        for (auto& c : candidates) {
            if (f == 0) {
                // First frame: use prior
                c.probability *= (c.frequency > 0.0) ? voicing : (1.0 - voicing);
            } else {
                // Viterbi: find best predecessor
                double bestPathProb = -1e30;
                int bestPrev = 0;
                for (int p = 0; p < m_trellis[f - 1].size(); ++p) {
                    double transP = transitionCost(
                        m_trellis[f - 1][p].frequency, c.frequency);
                    double pathProb = m_trellis[f - 1][p].probability * transP;
                    if (pathProb > bestPathProb) {
                        bestPathProb = pathProb;
                        bestPrev = p;
                    }
                }
                c.probability *= bestPathProb;
                c.prevCandidate = bestPrev;
            }
        }

        m_trellis[f] = candidates;
    }

    // Viterbi backtrace
    QVector<PitchFrame> result = viterbiBacktrace(m_trellis);

    // Compute average F0
    double f0Sum = 0.0;
    int voicedCount = 0;
    for (const auto& fr : result) {
        if (fr.voiced) { f0Sum += fr.frequency; voicedCount++; }
    }
    m_stats.avgF0 = (voicedCount > 0) ? f0Sum / voicedCount : 0.0;
    m_stats.numFrames = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit trajectoryCompleted(result.size(), m_stats.avgF0, timer.elapsed());
    return result;
}

/* ---- Get cepstrum ---- */

QVector<double> PitchDetector6::cepstrum() const { return m_lastCepstrum; }

/* ---- Get candidates ---- */

QVector<PitchDetector6::HMMCandidate> PitchDetector6::candidates() const
{
    return m_lastCandidates;
}

/* ---- Reset ---- */

void PitchDetector6::resetStatistics()
{
    m_lastCepstrum.clear();
    m_lastCandidates.clear();
    m_trellis.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
