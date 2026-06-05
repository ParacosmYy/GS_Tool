/**
 * @file algo_4642.cpp
 */
#include "poly4642/algo_4642.h"
QVector<double> algo_4642::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
