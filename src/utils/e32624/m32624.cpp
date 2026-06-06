#include "e32624/m32624.h"
QVector<double> m32624::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
