#include "r16237/m16237.h"
QVector<double> m16237::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
