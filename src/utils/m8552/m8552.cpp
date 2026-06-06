#include "m8552/m8552.h"
QVector<double> m8552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
