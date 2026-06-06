#include "k32850/m32850.h"
QVector<double> m32850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
