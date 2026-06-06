#include "l16011/m16011.h"
QVector<double> m16011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
