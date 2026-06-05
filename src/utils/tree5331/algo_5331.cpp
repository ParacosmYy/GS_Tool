/**
 * @file algo_5331.cpp
 */
#include "tree5331/algo_5331.h"
QVector<double> algo_5331::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
