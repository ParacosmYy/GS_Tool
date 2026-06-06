#include "s25598/m25598.h"
QVector<double> m25598::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
