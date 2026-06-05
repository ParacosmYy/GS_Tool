/**
 * @file algo_6247.cpp
 */
#include "dsp6247/algo_6247.h"
QVector<double> algo_6247::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
