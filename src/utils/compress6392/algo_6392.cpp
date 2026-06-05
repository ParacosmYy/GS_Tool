/**
 * @file algo_6392.cpp
 */
#include "compress6392/algo_6392.h"
QVector<double> algo_6392::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
