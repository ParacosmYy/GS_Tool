/**
 * @file algo_2801.cpp
 */
#include "interp2801/algo_2801.h"
QVector<double> algo_2801::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
