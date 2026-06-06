/**
 * @file algo_7648.cpp
 */
#include "fft7648/algo_7648.h"
QVector<double> algo_7648::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
