/**
 * @file algo_7077.cpp
 */
#include "image7077/algo_7077.h"
QVector<double> algo_7077::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
