/**
 * @file algo_7583.cpp
 */
#include "string7583/algo_7583.h"
QVector<double> algo_7583::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
