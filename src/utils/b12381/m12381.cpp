#include "b12381/m12381.h"
QVector<double> m12381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
