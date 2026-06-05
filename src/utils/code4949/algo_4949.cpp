/**
 * @file algo_4949.cpp
 */
#include "code4949/algo_4949.h"
QVector<double> algo_4949::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
