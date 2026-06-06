#include "k16450/m16450.h"
QVector<double> m16450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
