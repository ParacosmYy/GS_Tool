/**
 * @file AllpassFilter.cpp
 * @brief 全通滤波器实现 — 相位校正与级联延迟线
 */

#include "utils/dsp7/AllpassFilter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

AllpassFilter::AllpassFilter(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void AllpassFilter::addFirstOrderSection(double coefficient)
{
    AllpassSection sec;
    sec.coefficient = coefficient;
    sec.delay = 0;
    sec.x1 = 0.0;
    sec.y1 = 0.0;
    m_sections.append(sec);
    m_delayLines.append(QVector<double>{});
    m_delayWritePos.append(0);
    ++m_stats.totalSectionsCreated;
}

void AllpassFilter::addSecondOrderSection(double alpha, double beta)
{
    AllpassSection sec;
    sec.coefficient = alpha;
    sec.delay = beta;
    sec.x1 = 0.0;
    sec.x2 = 0.0;
    sec.y1 = 0.0;
    sec.y2 = 0.0;
    m_sections.append(sec);
    m_delayLines.append(QVector<double>{});
    m_delayWritePos.append(0);
    ++m_stats.totalSectionsCreated;
}

void AllpassFilter::addNestedSection(double coefficient, int delaySamples)
{
    AllpassSection sec;
    sec.coefficient = coefficient;
    sec.delay = delaySamples;
    sec.x1 = 0.0;
    m_sections.append(sec);

    /* 为嵌套节分配延迟线 */
    QVector<double> delayLine(delaySamples, 0.0);
    m_delayLines.append(delayLine);
    m_delayWritePos.append(0);
    ++m_stats.totalSectionsCreated;
}

double AllpassFilter::processSample(double input)
{
    double sample = input;

    for (int i = 0; i < m_sections.size(); ++i) {
        if (m_sections[i].delay > 1.0) {
            sample = processNested(m_sections[i], sample);
        } else if (m_sections[i].x2 != 0.0 || m_sections[i].y2 != 0.0) {
            sample = processSecondOrder(m_sections[i], sample);
        } else {
            sample = processFirstOrder(m_sections[i], sample);
        }
    }

    ++m_stats.totalSamplesProcessed;
    return sample;
}

QVector<double> AllpassFilter::processBuffer(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    int N = samples.size();
    QVector<double> output(N);

    for (int i = 0; i < N; ++i) {
        output[i] = processSample(samples[i]);
    }

    m_timeSum += timer.elapsed();
    if (m_stats.totalSamplesProcessed > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalSamplesProcessed / qMax(1, N));
    }

    return output;
}

double AllpassFilter::phaseResponse(double frequency) const
{
    double totalPhase = 0.0;
    for (const auto& sec : m_sections) {
        totalPhase += sectionPhase(sec, frequency);
    }
    return totalPhase;
}

double AllpassFilter::groupDelay(double frequency) const
{
    /* 数值微分法计算群延迟: τ = -dφ/dω */
    double df = 1e-4;
    double freqLow = qMax(0.0, frequency - df);
    double freqHigh = qMin(0.5, frequency + df);

    double phaseLow = phaseResponse(freqLow);
    double phaseHigh = phaseResponse(freqHigh);

    double gd = -(phaseHigh - phaseLow) / (2.0 * df * 2.0 * M_PI);
    return qFabs(gd);
}

QVector<QPair<double, double>> AllpassFilter::phaseResponseCurve(int numPoints) const
{
    QVector<QPair<double, double>> curve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double freq = 0.5 * i / (numPoints - 1);
        curve[i] = qMakePair(freq, phaseResponse(freq));
    }
    return curve;
}

void AllpassFilter::resetState()
{
    for (auto& sec : m_sections) {
        sec.x1 = 0.0;
        sec.x2 = 0.0;
        sec.y1 = 0.0;
        sec.y2 = 0.0;
    }
    for (auto& dl : m_delayLines) {
        dl.fill(0.0);
    }
    for (auto& pos : m_delayWritePos) {
        pos = 0;
    }
    ++m_stats.totalFiltersReset;
}

int AllpassFilter::sectionCount() const
{
    return m_sections.size();
}

AllpassFilter::Stats AllpassFilter::stats() const
{
    return m_stats;
}

void AllpassFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

double AllpassFilter::processFirstOrder(AllpassSection& section, double input)
{
    /* H(z) = (a + z^{-1}) / (1 + a*z^{-1}) */
    double a = section.coefficient;
    double output = a * input + section.x1 - a * section.y1;
    section.x1 = input;
    section.y1 = output;
    return output;
}

double AllpassFilter::processSecondOrder(AllpassSection& section, double input)
{
    /* H(z) = (a + b*z^{-1} + z^{-2}) / (1 + b*z^{-1} + a*z^{-2}) */
    double alpha = section.coefficient;
    double beta = section.delay;
    double output = alpha * input + beta * section.x1 + section.x2
                   - beta * section.y1 - alpha * section.y2;
    section.x2 = section.x1;
    section.x1 = input;
    section.y2 = section.y1;
    section.y1 = output;
    return output;
}

double AllpassFilter::processNested(AllpassSection& section, double input)
{
    /* 嵌套全通: y[n] = -a*x[n] + x[n-M] + a*y[n-M] */
    int delayLen = static_cast<int>(section.delay);
    int idx = m_delayWritePos[m_sections.indexOf(section)] % delayLen;

    int secIdx = -1;
    for (int i = 0; i < m_sections.size(); ++i) {
        if (&m_sections[i] == &section) { secIdx = i; break; }
    }
    if (secIdx < 0) return input;

    int writePos = m_delayWritePos[secIdx];
    int readPos = writePos;
    double delayedInput = m_delayLines[secIdx][readPos];

    double a = section.coefficient;
    double output = -a * input + delayedInput * (1.0 - a * a);
    double delayIn = input + a * output;

    m_delayLines[secIdx][writePos] = delayIn;
    m_delayWritePos[secIdx] = (writePos + 1) % delayLen;

    return output;
}

double AllpassFilter::sectionPhase(const AllpassSection& section, double freq) const
{
    double omega = 2.0 * M_PI * freq;

    if (section.delay > 1.0) {
        /* 嵌套全通近似相位: φ ≈ -ω + 2*arctan(a*sin(ωM) / (1 - a*cos(ωM))) */
        double M = section.delay;
        double a = section.coefficient;
        double denom = 1.0 - a * qCos(omega * M);
        double numer = a * qSin(omega * M);
        return -omega + 2.0 * qAtan2(numer, denom);
    }

    double a = section.coefficient;
    /* 一阶全通相位: φ = -ω + 2*arctan(a*sin(ω) / (1 + a*cos(ω))) */
    double denom = 1.0 + a * qCos(omega);
    double numer = a * qSin(omega);
    return -omega + 2.0 * qAtan2(numer, denom);
}
