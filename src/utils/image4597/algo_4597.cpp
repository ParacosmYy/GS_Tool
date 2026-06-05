/**
 * @file algo_4597.cpp
 */
#include "image4597/algo_4597.h"
QVector<double> algo_4597::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
