#include "b16581/m16581.h"
QVector<double> m16581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
