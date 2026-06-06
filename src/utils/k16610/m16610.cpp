#include "k16610/m16610.h"
QVector<double> m16610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
