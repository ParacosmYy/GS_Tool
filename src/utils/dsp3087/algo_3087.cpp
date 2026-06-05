/**
 * @file algo_3087.cpp
 */
#include "dsp3087/algo_3087.h"
QVector<double> algo_3087::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
