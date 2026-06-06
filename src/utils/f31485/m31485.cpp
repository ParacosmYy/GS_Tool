#include "f31485/m31485.h"
QVector<double> m31485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
