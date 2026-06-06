#include "e16584/m16584.h"
QVector<double> m16584::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
