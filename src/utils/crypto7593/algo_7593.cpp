/**
 * @file algo_7593.cpp
 */
#include "crypto7593/algo_7593.h"
QVector<double> algo_7593::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
