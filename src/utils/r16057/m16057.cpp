#include "r16057/m16057.h"
QVector<double> m16057::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
