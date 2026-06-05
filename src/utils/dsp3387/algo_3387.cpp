/**
 * @file algo_3387.cpp
 */
#include "dsp3387/algo_3387.h"
QVector<double> algo_3387::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
