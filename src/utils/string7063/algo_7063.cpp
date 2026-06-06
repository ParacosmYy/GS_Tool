/**
 * @file algo_7063.cpp
 */
#include "string7063/algo_7063.h"
QVector<double> algo_7063::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
