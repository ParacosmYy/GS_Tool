#include "r32057/m32057.h"
QVector<double> m32057::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
