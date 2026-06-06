/**
 * @file algo_7588.cpp
 */
#include "fft7588/algo_7588.h"
QVector<double> algo_7588::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
