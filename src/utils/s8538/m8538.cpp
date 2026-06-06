#include "s8538/m8538.h"
QVector<double> m8538::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
