#include "k10630/m10630.h"
QVector<double> m10630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
