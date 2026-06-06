#include "k16130/m16130.h"
QVector<double> m16130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
