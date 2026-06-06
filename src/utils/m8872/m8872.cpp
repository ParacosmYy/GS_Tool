#include "m8872/m8872.h"
QVector<double> m8872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
