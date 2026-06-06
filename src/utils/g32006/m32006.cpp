#include "g32006/m32006.h"
QVector<double> m32006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
