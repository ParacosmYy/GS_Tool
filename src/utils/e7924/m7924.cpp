#include "e7924/m7924.h"
QVector<double> m7924::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
