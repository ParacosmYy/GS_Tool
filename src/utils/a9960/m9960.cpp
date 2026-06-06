#include "a9960/m9960.h"
QVector<double> m9960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
