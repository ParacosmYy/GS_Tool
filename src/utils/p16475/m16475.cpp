#include "p16475/m16475.h"
QVector<double> m16475::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
