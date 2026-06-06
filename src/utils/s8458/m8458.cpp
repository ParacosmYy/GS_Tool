#include "s8458/m8458.h"
QVector<double> m8458::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
