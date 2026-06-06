#include "m26552/m26552.h"
QVector<double> m26552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
