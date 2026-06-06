#include "k16790/m16790.h"
QVector<double> m16790::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
