/**
 * @file algo_4813.cpp
 */
#include "crypto4813/algo_4813.h"
QVector<double> algo_4813::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
