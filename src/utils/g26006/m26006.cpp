#include "g26006/m26006.h"
QVector<double> m26006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
