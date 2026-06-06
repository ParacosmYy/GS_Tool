#include "n25553/m25553.h"
QVector<double> m25553::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
