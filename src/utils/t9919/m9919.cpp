#include "t9919/m9919.h"
QVector<double> m9919::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
