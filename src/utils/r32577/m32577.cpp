#include "r32577/m32577.h"
QVector<double> m32577::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
