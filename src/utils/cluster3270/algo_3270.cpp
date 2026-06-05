/**
 * @file algo_3270.cpp
 */
#include "cluster3270/algo_3270.h"
QVector<double> algo_3270::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
