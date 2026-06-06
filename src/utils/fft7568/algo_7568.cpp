/**
 * @file algo_7568.cpp
 */
#include "fft7568/algo_7568.h"
QVector<double> algo_7568::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
