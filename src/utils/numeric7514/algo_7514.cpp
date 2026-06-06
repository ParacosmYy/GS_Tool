/**
 * @file algo_7514.cpp
 */
#include "numeric7514/algo_7514.h"
QVector<double> algo_7514::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
