#include "b18621/m18621.h"
QVector<double> m18621::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
