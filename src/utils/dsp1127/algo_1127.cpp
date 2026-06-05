/**
 * @file algo_1127.cpp
 * @brief Algorithm module 1127
 */
#include "dsp1127/algo_1127.h"
QVector<double> algo_1127::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
