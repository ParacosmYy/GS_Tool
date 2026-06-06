#include "k15430/m15430.h"
QVector<double> m15430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
