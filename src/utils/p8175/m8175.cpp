#include "p8175/m8175.h"
QVector<double> m8175::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
