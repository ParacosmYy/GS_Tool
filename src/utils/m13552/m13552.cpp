#include "m13552/m13552.h"
QVector<double> m13552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
