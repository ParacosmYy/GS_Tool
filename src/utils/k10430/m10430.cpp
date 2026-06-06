#include "k10430/m10430.h"
QVector<double> m10430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
