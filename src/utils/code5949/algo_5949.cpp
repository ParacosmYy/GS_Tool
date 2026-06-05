/**
 * @file algo_5949.cpp
 */
#include "code5949/algo_5949.h"
QVector<double> algo_5949::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
