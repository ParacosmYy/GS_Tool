#include "k32370/m32370.h"
QVector<double> m32370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
