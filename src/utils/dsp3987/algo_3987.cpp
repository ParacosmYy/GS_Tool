/**
 * @file algo_3987.cpp
 */
#include "dsp3987/algo_3987.h"
QVector<double> algo_3987::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
