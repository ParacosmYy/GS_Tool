/**
 * @file algo_5125.cpp
 */
#include "matrix5125/algo_5125.h"
QVector<double> algo_5125::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
