#include "f33805/m33805.h"
QVector<double> m33805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
