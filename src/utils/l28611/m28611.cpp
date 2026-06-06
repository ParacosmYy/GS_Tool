#include "l28611/m28611.h"
QVector<double> m28611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
