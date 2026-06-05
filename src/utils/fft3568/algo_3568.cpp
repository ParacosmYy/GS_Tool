/**
 * @file algo_3568.cpp
 */
#include "fft3568/algo_3568.h"
QVector<double> algo_3568::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
