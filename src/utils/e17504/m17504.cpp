#include "e17504/m17504.h"
QVector<double> m17504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
