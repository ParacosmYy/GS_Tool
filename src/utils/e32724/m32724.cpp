#include "e32724/m32724.h"
QVector<double> m32724::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
