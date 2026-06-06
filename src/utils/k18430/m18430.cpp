#include "k18430/m18430.h"
QVector<double> m18430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
