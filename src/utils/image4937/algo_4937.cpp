/**
 * @file algo_4937.cpp
 */
#include "image4937/algo_4937.h"
QVector<double> algo_4937::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
