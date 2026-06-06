#include "b16901/m16901.h"
QVector<double> m16901::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
