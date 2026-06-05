/**
 * @file algo_3128.cpp
 */
#include "fft3128/algo_3128.h"
QVector<double> algo_3128::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
