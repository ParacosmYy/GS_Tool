#include "a32980/m32980.h"
QVector<double> m32980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
