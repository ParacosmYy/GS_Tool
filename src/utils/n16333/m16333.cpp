#include "n16333/m16333.h"
QVector<double> m16333::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
