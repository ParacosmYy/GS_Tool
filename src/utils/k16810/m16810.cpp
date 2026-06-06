#include "k16810/m16810.h"
QVector<double> m16810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
