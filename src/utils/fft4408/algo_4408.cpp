/**
 * @file algo_4408.cpp
 */
#include "fft4408/algo_4408.h"
QVector<double> algo_4408::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
