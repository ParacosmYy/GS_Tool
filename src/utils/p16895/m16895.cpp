#include "p16895/m16895.h"
QVector<double> m16895::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
