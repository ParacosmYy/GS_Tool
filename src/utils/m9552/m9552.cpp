#include "m9552/m9552.h"
QVector<double> m9552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
