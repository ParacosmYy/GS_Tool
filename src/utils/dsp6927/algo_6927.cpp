/**
 * @file algo_6927.cpp
 */
#include "dsp6927/algo_6927.h"
QVector<double> algo_6927::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
