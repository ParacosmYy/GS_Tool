#include "d18403/m18403.h"
QVector<double> m18403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
