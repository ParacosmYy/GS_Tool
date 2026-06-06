#include "m8752/m8752.h"
QVector<double> m8752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
