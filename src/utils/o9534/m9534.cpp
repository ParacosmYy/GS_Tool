#include "o9534/m9534.h"
QVector<double> m9534::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
