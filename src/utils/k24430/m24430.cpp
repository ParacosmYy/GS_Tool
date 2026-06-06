#include "k24430/m24430.h"
QVector<double> m24430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
