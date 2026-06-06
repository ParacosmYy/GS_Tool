/**
 * @file algo_7117.cpp
 */
#include "image7117/algo_7117.h"
QVector<double> algo_7117::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
