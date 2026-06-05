/**
 * @file algo_3278.cpp
 */
#include "neural3278/algo_3278.h"
QVector<double> algo_3278::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
