#include "r16577/m16577.h"
QVector<double> m16577::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
