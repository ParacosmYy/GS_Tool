/**
 * @file algo_7219.cpp
 */
#include "quantum7219/algo_7219.h"
QVector<double> algo_7219::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
