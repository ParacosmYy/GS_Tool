#include "l16151/m16151.h"
QVector<double> m16151::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
