#include "n16613/m16613.h"
QVector<double> m16613::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
