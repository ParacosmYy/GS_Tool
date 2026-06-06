#include "a12080/m12080.h"
QVector<double> m12080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
