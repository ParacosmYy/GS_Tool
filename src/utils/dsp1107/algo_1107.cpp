/**
 * @file algo_1107.cpp
 * @brief Algorithm module 1107
 */
#include "dsp1107/algo_1107.h"
QVector<double> algo_1107::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
