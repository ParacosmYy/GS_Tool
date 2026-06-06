#include "r16677/m16677.h"
QVector<double> m16677::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
