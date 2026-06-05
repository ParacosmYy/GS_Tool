/**
 * @file algo_3369.cpp
 */
#include "code3369/algo_3369.h"
QVector<double> algo_3369::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
