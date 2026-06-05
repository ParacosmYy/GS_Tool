/**
 * @file algo_5068.cpp
 */
#include "fft5068/algo_5068.h"
QVector<double> algo_5068::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
