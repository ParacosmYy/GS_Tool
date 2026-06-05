/**
 * @file algo_2828.cpp
 */
#include "fft2828/algo_2828.h"
QVector<double> algo_2828::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
