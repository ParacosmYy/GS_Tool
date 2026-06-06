#include "m8932/m8932.h"
QVector<double> m8932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
