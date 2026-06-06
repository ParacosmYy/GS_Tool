#include "e8304/m8304.h"
QVector<double> m8304::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
