/**
 * @file algo_5749.cpp
 */
#include "code5749/algo_5749.h"
QVector<double> algo_5749::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
