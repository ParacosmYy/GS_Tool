/**
 * @file algo_3107.cpp
 */
#include "dsp3107/algo_3107.h"
QVector<double> algo_3107::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
