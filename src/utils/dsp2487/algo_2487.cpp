/**
 * @file algo_2487.cpp
 * @brief Algorithm module 2487
 */
#include "dsp2487/algo_2487.h"
QVector<double> algo_2487::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
