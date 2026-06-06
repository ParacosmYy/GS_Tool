/**
 * @file algo_6952.cpp
 */
#include "compress6952/algo_6952.h"
QVector<double> algo_6952::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
