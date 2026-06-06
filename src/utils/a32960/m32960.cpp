#include "a32960/m32960.h"
QVector<double> m32960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
