/**
 * @file PhaseVocoder.cpp
 * @brief PhaseVocoder 实现
 *
 * 实现相位声码器：STFT分析/合成、瞬时频率估计、
 * 相位锁定和重叠相加(overlap-add)。
 */

#include "utils/dsp164/PhaseVocoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

PhaseVocoder::PhaseVocoder(QObject* parent)
    : QObject(parent)
{
}

PhaseVocoder::~PhaseVocoder() = default;

void PhaseVocoder::setWindowSize(int size)
{
    /* Must be power of 2 */
    int p = 1;
    while (p < size) p <<= 1;
    m_windowSize = p;
}

void PhaseVocoder::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

void PhaseVocoder::setWindowType(int type)
{
    m_windowType = qBound(0, type, 1);
}

QVector<double> PhaseVocoder::createWindow(int size) const
{
    QVector<double> win(size);
    for (int i = 0; i < size; ++i) {
        if (m_windowType == 0) {
            /* Hann window */
            win[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / size));
        } else {
            /* Hamming window */
            win[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / size);
        }
    }
    return win;
}

void PhaseVocoder::analyzeFrame(const QVector<double>& frame,
                                QVector<double>& magnitude,
                                QVector<double>& phase) const
{
    const int N = frame.size();
    magnitude.resize(N / 2 + 1);
    phase.resize(N / 2 + 1);

    /* Simple DFT for the analysis frame */
    for (int k = 0; k <= N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im -= frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
        phase[k] = qAtan2(im, re);
    }
}

void PhaseVocoder::computeInstantFreq(const QVector<double>& phase,
                                      const QVector<double>& prevPhase,
                                      double hop,
                                      QVector<double>& instFreq) const
{
    const int bins = phase.size();
    instFreq.resize(bins);
    for (int k = 0; k < bins; ++k) {
        double omega = 2.0 * M_PI * k / (2 * (bins - 1));
        double delta = phase[k] - prevPhase[k] - omega * hop;
        /* Wrap to [-pi, pi] */
        while (delta > M_PI) delta -= 2.0 * M_PI;
        while (delta < -M_PI) delta += 2.0 * M_PI;
        instFreq[k] = omega + delta / hop;
    }
}

void PhaseVocoder::phaseLock(QVector<double>& phase,
                             const QVector<double>& prevPhase,
                             const QVector<double>& magnitude,
                             double omega, double ratio) const
{
    /* Phase locking: keep phase coherent across bins */
    int bins = phase.size();
    for (int k = 0; k < bins; ++k) {
        double expectedPhase = prevPhase[k] + omega * k * m_hopSize * ratio;
        double delta = phase[k] - expectedPhase;
        while (delta > M_PI) delta -= 2.0 * M_PI;
        while (delta < -M_PI) delta += 2.0 * M_PI;
        phase[k] = expectedPhase + delta;
    }
}

void PhaseVocoder::synthesizeFrame(const QVector<double>& magnitude,
                                   const QVector<double>& phase,
                                   int synthesisHop,
                                   QVector<double>& output, int outPos) const
{
    const int N = magnitude.size() * 2 - 2;
    if (N <= 0) return;

    /* Inverse DFT */
    QVector<double> frame(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double val = 0.0;
        for (int k = 0; k <= N / 2; ++k) {
            double angle = phase[k] + 2.0 * M_PI * k * n / N;
            val += magnitude[k] * qCos(angle);
        }
        frame[n] = val / N;
    }

    /* Apply synthesis window */
    QVector<double> win = createWindow(N);
    QVector<double> winSum(N, 0.0);
    for (int i = 0; i < N; ++i) {
        winSum[i] = win[i] * win[i];
        if (winSum[i] < 1e-10) winSum[i] = 1e-10;
    }

    overlapAdd(output, frame, outPos, winSum);
}

void PhaseVocoder::overlapAdd(QVector<double>& output,
                              const QVector<double>& frame,
                              int pos, const QVector<double>& winSum) const
{
    int len = frame.size();
    for (int i = 0; i < len; ++i) {
        int idx = pos + i;
        if (idx >= 0 && idx < output.size()) {
            output[idx] += frame[i];
        }
    }
}

QVector<double> PhaseVocoder::timeStretch(const QVector<double>& signal,
                                          double stretchRatio)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || stretchRatio <= 0) return QVector<double>();

    int N = m_windowSize;
    int analysisHop = m_hopSize;
    int synthesisHop = qMax(1, qRound(analysisHop * stretchRatio));

    int numFrames = qMax(1, (signal.size() - N) / analysisHop + 1);
    int outputLen = numFrames * synthesisHop + N;
    QVector<double> output(outputLen, 0.0);

    QVector<double> win = createWindow(N);
    QVector<double> prevPhase(N / 2 + 1, 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * analysisHop;
        QVector<double> frame(N, 0.0);
        for (int i = 0; i < N && (start + i) < signal.size(); ++i) {
            frame[i] = signal[start + i] * win[i];
        }

        QVector<double> mag, ph;
        analyzeFrame(frame, mag, ph);

        /* Phase accumulation with instantaneous frequency */
        QVector<double> instFreq;
        computeInstantFreq(ph, prevPhase, analysisHop, instFreq);

        /* Accumulate phase for synthesis hop */
        QVector<double> synthPhase(ph.size());
        for (int k = 0; k < ph.size(); ++k) {
            synthPhase[k] = prevPhase[k] + instFreq[k] * synthesisHop;
        }

        phaseLock(synthPhase, prevPhase, mag, 2.0 * M_PI / N, stretchRatio);

        int outPos = f * synthesisHop;
        synthesizeFrame(mag, synthPhase, synthesisHop, output, outPos);

        prevPhase = synthPhase;
    }

    /* Trim output */
    int finalLen = qRound(signal.size() * stretchRatio);
    if (finalLen > 0 && finalLen < output.size()) {
        output.resize(finalLen);
    }

    m_stats.totalTimeStretches++;
    m_stats.lastFrameCount = numFrames;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalTimeStretches + m_stats.totalPitchShifts;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit timeStretchCompleted(stretchRatio);
    return output;
}

QVector<double> PhaseVocoder::pitchShift(const QVector<double>& signal,
                                         double semitones)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return QVector<double>();

    double ratio = qPow(2.0, semitones / 12.0);
    /* Pitch shift = time stretch by ratio, then resample by 1/ratio */
    QVector<double> stretched = timeStretch(signal, ratio);

    /* Simple resampling: linear interpolation */
    int newLen = qRound(signal.size());
    QVector<double> result(newLen, 0.0);
    for (int i = 0; i < newLen; ++i) {
        double srcIdx = i * (stretched.size() - 1.0) / qMax(1, newLen - 1);
        int idx = qFloor(srcIdx);
        double frac = srcIdx - idx;
        if (idx + 1 < stretched.size()) {
            result[i] = stretched[idx] * (1.0 - frac) + stretched[idx + 1] * frac;
        } else if (idx < stretched.size()) {
            result[i] = stretched[idx];
        }
    }

    m_stats.totalPitchShifts++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalTimeStretches + m_stats.totalPitchShifts;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit pitchShiftCompleted(semitones);
    return result;
}

void PhaseVocoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
