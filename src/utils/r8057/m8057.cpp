#include "r8057/m8057.h"
QVector<double> m8057::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
