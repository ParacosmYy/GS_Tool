#include "e16864/m16864.h"
QVector<double> m16864::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
