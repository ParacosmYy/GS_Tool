/**
 * @file algo_3767.cpp
 */
#include "dsp3767/algo_3767.h"
QVector<double> algo_3767::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
