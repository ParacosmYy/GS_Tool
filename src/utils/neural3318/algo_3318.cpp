/**
 * @file algo_3318.cpp
 */
#include "neural3318/algo_3318.h"
QVector<double> algo_3318::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
