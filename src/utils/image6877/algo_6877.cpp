/**
 * @file algo_6877.cpp
 */
#include "image6877/algo_6877.h"
QVector<double> algo_6877::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
