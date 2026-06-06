#include "t16119/m16119.h"
QVector<double> m16119::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
