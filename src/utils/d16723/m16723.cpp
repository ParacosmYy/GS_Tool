#include "d16723/m16723.h"
QVector<double> m16723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
