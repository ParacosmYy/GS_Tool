/**
 * @file algo_6313.cpp
 */
#include "crypto6313/algo_6313.h"
QVector<double> algo_6313::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
