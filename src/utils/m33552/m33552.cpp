#include "m33552/m33552.h"
QVector<double> m33552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
