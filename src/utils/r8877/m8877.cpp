#include "r8877/m8877.h"
QVector<double> m8877::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
