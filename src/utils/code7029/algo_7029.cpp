/**
 * @file algo_7029.cpp
 */
#include "code7029/algo_7029.h"
QVector<double> algo_7029::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
