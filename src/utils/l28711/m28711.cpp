#include "l28711/m28711.h"
QVector<double> m28711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
