/**
 * @file algo_7508.cpp
 */
#include "fft7508/algo_7508.h"
QVector<double> algo_7508::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
