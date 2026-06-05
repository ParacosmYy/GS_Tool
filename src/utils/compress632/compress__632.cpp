/**
 * @file compress__632.cpp
 * @brief compress__632 implementation
 */
#include "compress632/compress__632.h"
QVector<double> compress__632::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

