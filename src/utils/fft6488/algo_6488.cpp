/**
 * @file algo_6488.cpp
 */
#include "fft6488/algo_6488.h"
QVector<double> algo_6488::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
