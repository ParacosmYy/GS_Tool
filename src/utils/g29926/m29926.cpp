#include "g29926/m29926.h"
QVector<double> m29926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
