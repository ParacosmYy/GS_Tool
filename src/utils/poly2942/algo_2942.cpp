/**
 * @file algo_2942.cpp
 */
#include "poly2942/algo_2942.h"
QVector<double> algo_2942::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
