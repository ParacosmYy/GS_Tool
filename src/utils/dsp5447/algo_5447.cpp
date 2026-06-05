/**
 * @file algo_5447.cpp
 */
#include "dsp5447/algo_5447.h"
QVector<double> algo_5447::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
