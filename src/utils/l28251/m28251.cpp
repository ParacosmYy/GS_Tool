#include "l28251/m28251.h"
QVector<double> m28251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
