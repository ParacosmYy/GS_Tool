/**
 * @file algo_4553.cpp
 */
#include "crypto4553/algo_4553.h"
QVector<double> algo_4553::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
