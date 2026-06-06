#include "g16406/m16406.h"
QVector<double> m16406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
