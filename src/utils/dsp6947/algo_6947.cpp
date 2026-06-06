/**
 * @file algo_6947.cpp
 */
#include "dsp6947/algo_6947.h"
QVector<double> algo_6947::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
