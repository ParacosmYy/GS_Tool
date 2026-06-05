/**
 * @file algo_6514.cpp
 */
#include "numeric6514/algo_6514.h"
QVector<double> algo_6514::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
