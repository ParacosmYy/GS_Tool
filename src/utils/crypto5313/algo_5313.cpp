/**
 * @file algo_5313.cpp
 */
#include "crypto5313/algo_5313.h"
QVector<double> algo_5313::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
