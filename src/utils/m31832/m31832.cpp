#include "m31832/m31832.h"
QVector<double> m31832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
