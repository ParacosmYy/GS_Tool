#include "c16282/m16282.h"
QVector<double> m16282::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
