/**
 * @file algo_5937.cpp
 */
#include "image5937/algo_5937.h"
QVector<double> algo_5937::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
