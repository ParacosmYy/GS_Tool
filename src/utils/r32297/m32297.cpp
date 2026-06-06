#include "r32297/m32297.h"
QVector<double> m32297::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
