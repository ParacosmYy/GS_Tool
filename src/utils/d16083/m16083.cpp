#include "d16083/m16083.h"
QVector<double> m16083::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
