#include "g32226/m32226.h"
QVector<double> m32226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
