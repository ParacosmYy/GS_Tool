/**
 * @file algo_6523.cpp
 */
#include "string6523/algo_6523.h"
QVector<double> algo_6523::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
