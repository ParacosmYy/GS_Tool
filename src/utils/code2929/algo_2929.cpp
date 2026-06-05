/**
 * @file algo_2929.cpp
 */
#include "code2929/algo_2929.h"
QVector<double> algo_2929::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
