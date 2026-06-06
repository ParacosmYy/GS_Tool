#include "m18552/m18552.h"
QVector<double> m18552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
