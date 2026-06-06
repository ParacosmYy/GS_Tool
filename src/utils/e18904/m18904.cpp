#include "e18904/m18904.h"
QVector<double> m18904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
