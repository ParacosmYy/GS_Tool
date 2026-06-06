#include "l16031/m16031.h"
QVector<double> m16031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
