/**
 * @file algo_887.cpp
 * @brief Algorithm module 887
 */
#include "dsp887/algo_887.h"
QVector<double> algo_887::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
