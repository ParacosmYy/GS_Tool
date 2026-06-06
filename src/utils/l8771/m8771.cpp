#include "l8771/m8771.h"
QVector<double> m8771::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
