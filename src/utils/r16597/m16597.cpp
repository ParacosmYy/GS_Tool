#include "r16597/m16597.h"
QVector<double> m16597::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
