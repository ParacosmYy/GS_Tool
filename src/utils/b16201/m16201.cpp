#include "b16201/m16201.h"
QVector<double> m16201::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
