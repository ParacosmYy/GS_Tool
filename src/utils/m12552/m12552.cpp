#include "m12552/m12552.h"
QVector<double> m12552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
