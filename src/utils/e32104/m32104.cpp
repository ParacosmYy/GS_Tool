#include "e32104/m32104.h"
QVector<double> m32104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
