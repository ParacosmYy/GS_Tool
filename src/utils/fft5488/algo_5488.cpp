/**
 * @file algo_5488.cpp
 */
#include "fft5488/algo_5488.h"
QVector<double> algo_5488::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
