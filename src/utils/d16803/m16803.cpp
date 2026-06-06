#include "d16803/m16803.h"
QVector<double> m16803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
