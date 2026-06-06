#include "l16611/m16611.h"
QVector<double> m16611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
