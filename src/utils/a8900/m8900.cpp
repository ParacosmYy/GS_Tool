#include "a8900/m8900.h"
QVector<double> m8900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
