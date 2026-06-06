/**
 * @file algo_7097.cpp
 */
#include "image7097/algo_7097.h"
QVector<double> algo_7097::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
