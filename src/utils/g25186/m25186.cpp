#include "g25186/m25186.h"
QVector<double> m25186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
