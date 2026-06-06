#include "a9360/m9360.h"
QVector<double> m9360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
