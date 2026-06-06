#include "b18061/m18061.h"
QVector<double> m18061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
