#include "a8960/m8960.h"
QVector<double> m8960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
