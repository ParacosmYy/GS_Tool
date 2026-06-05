/**
 * @file algo_2933.cpp
 */
#include "crypto2933/algo_2933.h"
QVector<double> algo_2933::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
