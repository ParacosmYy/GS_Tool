#include "e32524/m32524.h"
QVector<double> m32524::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
