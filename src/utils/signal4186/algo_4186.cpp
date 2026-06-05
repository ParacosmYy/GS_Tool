/**
 * @file algo_4186.cpp
 */
#include "signal4186/algo_4186.h"
QVector<double> algo_4186::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
