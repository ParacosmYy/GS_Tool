/**
 * @file algo_2967.cpp
 */
#include "dsp2967/algo_2967.h"
QVector<double> algo_2967::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
