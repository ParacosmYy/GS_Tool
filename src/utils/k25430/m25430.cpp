#include "k25430/m25430.h"
QVector<double> m25430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
