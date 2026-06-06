#include "d16003/m16003.h"
QVector<double> m16003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
