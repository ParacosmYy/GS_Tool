#include "g16006/m16006.h"
QVector<double> m16006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
