#include "m8952/m8952.h"
QVector<double> m8952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
