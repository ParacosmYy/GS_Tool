/**
 * @file algo_3969.cpp
 */
#include "code3969/algo_3969.h"
QVector<double> algo_3969::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
