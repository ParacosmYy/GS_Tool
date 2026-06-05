/**
 * @file SparseArray.cpp
 * @brief 稀疏数组实现
 */

#include "utils/sparsae/SparseArray.h"

#include <QElapsedTimer>
#include <cmath>

SparseArray::SparseArray(int logicalSize, QObject* parent)
    : QObject(parent), m_logicalSize(logicalSize), m_timeSum(0.0)
{
    m_stats.logicalSize = logicalSize;
    updateSparsity();
}

void SparseArray::setValue(int index, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_logicalSize) return;

    if (std::abs(value) < 1e-15) {
        m_data.remove(index);
    } else {
        m_data[index] = value;
    }

    m_stats.totalAccesses++;
    m_stats.totalNonZeroElements = m_data.size();
    updateSparsity();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAccesses, 1ULL);

    emit valueSet(index, value);
}

double SparseArray::getValue(int index) const
{
    m_stats.totalAccesses++;
    if (index < 0 || index >= m_logicalSize) return 0.0;
    return m_data.value(index, 0.0);
}

void SparseArray::addValue(int index, double delta)
{
    if (index < 0 || index >= m_logicalSize) return;
    double current = m_data.value(index, 0.0);
    setValue(index, current + delta);
}

QVector<SparseArray::Element> SparseArray::nonZeroElements() const
{
    QVector<Element> result;
    result.reserve(m_data.size());
    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        result.append({it.key(), it.value()});
    }
    return result;
}

QVector<double> SparseArray::toDense() const
{
    QVector<double> result(m_logicalSize, 0.0);
    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        if (it.key() < m_logicalSize) {
            result[it.key()] = it.value();
        }
    }
    return result;
}

void SparseArray::fromDense(const QVector<double>& data, double threshold)
{
    QElapsedTimer timer;
    timer.start();

    m_data.clear();
    m_logicalSize = data.size();
    m_stats.logicalSize = m_logicalSize;

    for (int i = 0; i < data.size(); ++i) {
        if (std::abs(data[i]) > threshold) {
            m_data[i] = data[i];
        }
    }

    m_stats.totalNonZeroElements = m_data.size();
    updateSparsity();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAccesses, 1ULL);
}

double SparseArray::compressionRatio() const
{
    if (m_logicalSize == 0) return 1.0;
    /* 原始大小 vs 稀疏存储大小(每个非零元素存index+value) */
    double original = static_cast<double>(m_logicalSize) * sizeof(double);
    double sparse = static_cast<double>(m_data.size()) * (sizeof(int) + sizeof(double));
    return (original > 0) ? sparse / original : 1.0;
}

void SparseArray::prune(double threshold)
{
    QElapsedTimer timer;
    timer.start();

    int removed = 0;
    QVector<int> toRemove;
    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        if (std::abs(it.value()) < threshold) {
            toRemove.append(it.key());
        }
    }
    for (int idx : toRemove) {
        m_data.remove(idx);
        ++removed;
    }

    m_stats.totalNonZeroElements = m_data.size();
    updateSparsity();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAccesses, 1ULL);

    emit pruned(removed);
}

void SparseArray::updateSparsity()
{
    m_stats.sparsity = (m_logicalSize > 0)
        ? 1.0 - static_cast<double>(m_data.size()) / m_logicalSize
        : 1.0;
}

void SparseArray::resetStatistics()
{
    m_stats = Stats{};
    m_stats.logicalSize = m_logicalSize;
    m_stats.totalNonZeroElements = m_data.size();
    updateSparsity();
    m_timeSum = 0.0;
}
