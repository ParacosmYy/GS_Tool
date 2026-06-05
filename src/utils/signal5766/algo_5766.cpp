/**
 * @file algo_5766.cpp
 */
#include "signal5766/algo_5766.h"
QVector<double> algo_5766::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
