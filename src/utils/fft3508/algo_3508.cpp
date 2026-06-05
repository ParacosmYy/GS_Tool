/**
 * @file algo_3508.cpp
 */
#include "fft3508/algo_3508.h"
QVector<double> algo_3508::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
