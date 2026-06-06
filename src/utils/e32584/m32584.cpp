#include "e32584/m32584.h"
QVector<double> m32584::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
