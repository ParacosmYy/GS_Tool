#include "l8431/m8431.h"
QVector<double> m8431::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
