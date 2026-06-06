#include "q16816/m16816.h"
QVector<double> m16816::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
