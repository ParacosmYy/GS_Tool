/**
 * @file algo_6843.cpp
 */
#include "string6843/algo_6843.h"
QVector<double> algo_6843::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
