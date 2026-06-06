#include "m8832/m8832.h"
QVector<double> m8832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
