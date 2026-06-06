#include "h16807/m16807.h"
QVector<double> m16807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
