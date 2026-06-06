/**
 * @file algo_7257.cpp
 */
#include "image7257/algo_7257.h"
QVector<double> algo_7257::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
