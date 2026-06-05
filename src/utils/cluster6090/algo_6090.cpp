/**
 * @file algo_6090.cpp
 */
#include "cluster6090/algo_6090.h"
QVector<double> algo_6090::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
