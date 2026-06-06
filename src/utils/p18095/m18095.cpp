#include "p18095/m18095.h"
QVector<double> m18095::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
