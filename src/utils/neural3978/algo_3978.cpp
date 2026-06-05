/**
 * @file algo_3978.cpp
 */
#include "neural3978/algo_3978.h"
QVector<double> algo_3978::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
