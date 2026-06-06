#include "k32530/m32530.h"
QVector<double> m32530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
