/**
 * @file algo_6452.cpp
 */
#include "compress6452/algo_6452.h"
QVector<double> algo_6452::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
