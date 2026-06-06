#include "r8457/m8457.h"
QVector<double> m8457::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
