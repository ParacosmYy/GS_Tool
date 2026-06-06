#include "m31712/m31712.h"
QVector<double> m31712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
