/**
 * @file algo_6632.cpp
 */
#include "compress6632/algo_6632.h"
QVector<double> algo_6632::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
