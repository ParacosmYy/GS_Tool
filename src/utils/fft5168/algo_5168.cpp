/**
 * @file algo_5168.cpp
 */
#include "fft5168/algo_5168.h"
QVector<double> algo_5168::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
