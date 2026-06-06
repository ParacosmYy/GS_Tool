#include "k32950/m32950.h"
QVector<double> m32950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
