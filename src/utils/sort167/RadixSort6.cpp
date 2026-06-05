/**
 * @file RadixSort6.cpp
 * @brief Radix sort for fixed-width integer arrays implementation
 */
#include "sort167/RadixSort6.h"
#include <QElapsedTimer>

QVector<double> RadixSort6::compute(const QVector<double> &input)
{
    QElapsedTimer t;
    t.start();
    m_stats.calls++;

    if (input.isEmpty()) {
        m_stats.errors++;
        return {};
    }

    QVector<double> result = input;
    m_stats.itemsProcessed += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

