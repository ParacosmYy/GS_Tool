#include "o16754/m16754.h"
QVector<double> m16754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
