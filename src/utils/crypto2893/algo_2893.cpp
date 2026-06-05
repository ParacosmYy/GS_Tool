/**
 * @file algo_2893.cpp
 */
#include "crypto2893/algo_2893.h"
QVector<double> algo_2893::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
