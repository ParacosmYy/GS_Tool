#include "f9505/m9505.h"
QVector<double> m9505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
