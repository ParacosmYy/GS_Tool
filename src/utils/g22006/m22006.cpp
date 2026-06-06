#include "g22006/m22006.h"
QVector<double> m22006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
