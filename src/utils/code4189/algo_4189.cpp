/**
 * @file algo_4189.cpp
 */
#include "code4189/algo_4189.h"
QVector<double> algo_4189::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
