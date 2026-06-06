#include "k16430/m16430.h"
QVector<double> m16430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
