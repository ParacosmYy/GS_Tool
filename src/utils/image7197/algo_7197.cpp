/**
 * @file algo_7197.cpp
 */
#include "image7197/algo_7197.h"
QVector<double> algo_7197::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
