#include "k16970/m16970.h"
QVector<double> m16970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
