#include "f8145/m8145.h"
QVector<double> m8145::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
