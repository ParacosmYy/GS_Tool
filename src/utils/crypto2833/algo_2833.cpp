/**
 * @file algo_2833.cpp
 */
#include "crypto2833/algo_2833.h"
QVector<double> algo_2833::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
