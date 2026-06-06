#include "g16026/m16026.h"
QVector<double> m16026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
