/**
 * @file algo_1306.cpp
 * @brief Algorithm module 1306
 */
#include "signal1306/algo_1306.h"
QVector<double> algo_1306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
