/**
 * @file algo_6369.cpp
 */
#include "code6369/algo_6369.h"
QVector<double> algo_6369::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
