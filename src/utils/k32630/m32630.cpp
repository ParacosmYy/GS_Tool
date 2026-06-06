#include "k32630/m32630.h"
QVector<double> m32630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
