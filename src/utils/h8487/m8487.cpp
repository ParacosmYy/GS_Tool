#include "h8487/m8487.h"
QVector<double> m8487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
