#include "a21380/m21380.h"
QVector<double> m21380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
