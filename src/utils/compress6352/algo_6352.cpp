/**
 * @file algo_6352.cpp
 */
#include "compress6352/algo_6352.h"
QVector<double> algo_6352::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
