#include "r16377/m16377.h"
QVector<double> m16377::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
