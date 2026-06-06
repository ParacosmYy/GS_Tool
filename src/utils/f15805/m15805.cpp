#include "f15805/m15805.h"
QVector<double> m15805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
