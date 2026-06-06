#include "p8895/m8895.h"
QVector<double> m8895::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
