#include "m31552/m31552.h"
QVector<double> m31552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
