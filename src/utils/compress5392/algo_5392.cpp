/**
 * @file algo_5392.cpp
 */
#include "compress5392/algo_5392.h"
QVector<double> algo_5392::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
