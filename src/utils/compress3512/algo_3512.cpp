/**
 * @file algo_3512.cpp
 */
#include "compress3512/algo_3512.h"
QVector<double> algo_3512::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
