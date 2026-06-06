#include "k32170/m32170.h"
QVector<double> m32170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
