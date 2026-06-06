#include "g16646/m16646.h"
QVector<double> m16646::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
