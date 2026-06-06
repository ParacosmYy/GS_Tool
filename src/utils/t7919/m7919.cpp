#include "t7919/m7919.h"
QVector<double> m7919::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
