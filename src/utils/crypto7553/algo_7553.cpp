/**
 * @file algo_7553.cpp
 */
#include "crypto7553/algo_7553.h"
QVector<double> algo_7553::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
