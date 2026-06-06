#include "s16218/m16218.h"
QVector<double> m16218::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
