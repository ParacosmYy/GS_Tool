#include "g10946/m10946.h"
QVector<double> m10946::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
