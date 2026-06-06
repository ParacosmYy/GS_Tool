#include "k32710/m32710.h"
QVector<double> m32710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
