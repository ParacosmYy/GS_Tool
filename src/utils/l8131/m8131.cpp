#include "l8131/m8131.h"
QVector<double> m8131::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
