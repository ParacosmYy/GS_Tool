#include "l36611/m36611.h"
QVector<double> m36611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
