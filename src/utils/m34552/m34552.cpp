#include "m34552/m34552.h"
QVector<double> m34552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
