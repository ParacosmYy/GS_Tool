#include "f16705/m16705.h"
QVector<double> m16705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
