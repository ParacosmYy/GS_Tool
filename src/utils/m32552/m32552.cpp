#include "m32552/m32552.h"
QVector<double> m32552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
