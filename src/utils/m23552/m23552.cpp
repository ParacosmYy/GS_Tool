#include "m23552/m23552.h"
QVector<double> m23552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
