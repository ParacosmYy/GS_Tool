#include "n16653/m16653.h"
QVector<double> m16653::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
