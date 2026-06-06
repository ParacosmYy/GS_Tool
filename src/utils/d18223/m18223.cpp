#include "d18223/m18223.h"
QVector<double> m18223::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
