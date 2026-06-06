#include "d16383/m16383.h"
QVector<double> m16383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
