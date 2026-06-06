#include "e32764/m32764.h"
QVector<double> m32764::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
