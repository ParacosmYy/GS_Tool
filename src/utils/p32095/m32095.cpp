#include "p32095/m32095.h"
QVector<double> m32095::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
