#include "t16899/m16899.h"
QVector<double> m16899::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
