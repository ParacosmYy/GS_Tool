/**
 * @file algo_3748.cpp
 */
#include "fft3748/algo_3748.h"
QVector<double> algo_3748::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
