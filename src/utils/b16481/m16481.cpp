#include "b16481/m16481.h"
QVector<double> m16481::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
