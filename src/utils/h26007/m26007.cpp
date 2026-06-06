#include "h26007/m26007.h"
QVector<double> m26007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
