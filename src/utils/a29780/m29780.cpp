#include "a29780/m29780.h"
QVector<double> m29780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
