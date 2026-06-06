#include "l7971/m7971.h"
QVector<double> m7971::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
