/**
 * @file StreamSegmenter.cpp
 * @brief 数据流分段器实现
 */

#include "utils/segmenter/StreamSegmenter.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

StreamSegmenter::StreamSegmenter(QObject* parent)
    : QObject(parent), m_strategy(Strategy::FixedWindow),
      m_windowSize(100), m_jumpThreshold(3.0),
      m_idleGapThreshold(100.0), m_customMarker(0.0),
      m_timeSum(0.0) {}

void StreamSegmenter::setStrategy(Strategy s) { m_strategy = s; }
void StreamSegmenter::setWindowSize(int size) { m_windowSize = qMax(1, size); }
void StreamSegmenter::setJumpThreshold(double t) { m_jumpThreshold = t; }
void StreamSegmenter::setIdleGapThreshold(double g) { m_idleGapThreshold = g; }
void StreamSegmenter::setCustomMarker(double m) { m_customMarker = m; }

QList<StreamSegmenter::Segment> StreamSegmenter::segment(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<Segment> result;
    if (data.isEmpty()) return result;

    switch (m_strategy) {
    case Strategy::FixedWindow:   result = fixedWindow(data); break;
    case Strategy::ThresholdJump: result = thresholdJump(data); break;
    case Strategy::IdleGap:       result = idleGap(data); break;
    case Strategy::CustomMarker:  result = customMarker(data); break;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalSegments;
    m_stats.totalPointsProcessed += data.size();
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalSegments;

    if (!result.isEmpty()) {
        int totalLen = 0;
        m_stats.maxSegmentLength = 0;
        m_stats.minSegmentLength = std::numeric_limits<int>::max();
        for (const auto& seg : result) {
            int len = seg.endIndex - seg.startIndex + 1;
            totalLen += len;
            if (len > m_stats.maxSegmentLength) m_stats.maxSegmentLength = len;
            if (len < m_stats.minSegmentLength) m_stats.minSegmentLength = len;
        }
        m_stats.avgSegmentLength = static_cast<double>(totalLen) / result.size();
    }

    emit segmentationComplete(result.size());
    return result;
}

QList<StreamSegmenter::Segment> StreamSegmenter::fixedWindow(const QVector<double>& data)
{
    QList<Segment> result;
    for (int i = 0; i < data.size(); i += m_windowSize) {
        int end = qMin(i + m_windowSize - 1, data.size() - 1);
        Segment seg = computeSegment(data, i, end);
        result.append(seg);
        emit segmentFound(seg);
    }
    return result;
}

QList<StreamSegmenter::Segment> StreamSegmenter::thresholdJump(const QVector<double>& data)
{
    QList<Segment> result;
    int segStart = 0;

    for (int i = 1; i < data.size(); ++i) {
        double jump = qAbs(data[i] - data[i - 1]);
        if (jump > m_jumpThreshold) {
            Segment seg = computeSegment(data, segStart, i - 1);
            result.append(seg);
            emit segmentFound(seg);
            segStart = i;
        }
    }

    if (segStart < data.size()) {
        Segment seg = computeSegment(data, segStart, data.size() - 1);
        result.append(seg);
        emit segmentFound(seg);
    }
    return result;
}

QList<StreamSegmenter::Segment> StreamSegmenter::idleGap(const QVector<double>& data)
{
    QList<Segment> result;
    if (data.size() < 2) {
        if (!data.isEmpty()) {
            Segment seg = computeSegment(data, 0, 0);
            result.append(seg);
        }
        return result;
    }

    int segStart = 0;
    for (int i = 1; i < data.size(); ++i) {
        if (qAbs(data[i] - data[i - 1]) > m_idleGapThreshold) {
            Segment seg = computeSegment(data, segStart, i - 1);
            result.append(seg);
            emit segmentFound(seg);
            segStart = i;
        }
    }

    if (segStart < data.size()) {
        Segment seg = computeSegment(data, segStart, data.size() - 1);
        result.append(seg);
        emit segmentFound(seg);
    }
    return result;
}

QList<StreamSegmenter::Segment> StreamSegmenter::customMarker(const QVector<double>& data)
{
    QList<Segment> result;
    int segStart = 0;

    for (int i = 0; i < data.size(); ++i) {
        if (qFuzzyCompare(data[i], m_customMarker) && i > segStart) {
            Segment seg = computeSegment(data, segStart, i - 1);
            result.append(seg);
            emit segmentFound(seg);
            segStart = i + 1;
        }
    }

    if (segStart < data.size()) {
        Segment seg = computeSegment(data, segStart, data.size() - 1);
        result.append(seg);
        emit segmentFound(seg);
    }
    return result;
}

StreamSegmenter::Segment StreamSegmenter::computeSegment(const QVector<double>& data, int start, int end)
{
    Segment seg;
    seg.startIndex = start;
    seg.endIndex = end;
    seg.mean = 0.0;
    seg.stddev = 0.0;
    seg.min = data[start];
    seg.max = data[start];

    double sum = 0.0;
    for (int i = start; i <= end; ++i) {
        sum += data[i];
        if (data[i] < seg.min) seg.min = data[i];
        if (data[i] > seg.max) seg.max = data[i];
    }
    int count = end - start + 1;
    seg.mean = sum / count;

    double sqSum = 0.0;
    for (int i = start; i <= end; ++i) {
        double d = data[i] - seg.mean;
        sqSum += d * d;
    }
    seg.stddev = qSqrt(sqSum / count);
    return seg;
}

void StreamSegmenter::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
