#include "j16909/m16909.h"
QVector<double> m16909::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
