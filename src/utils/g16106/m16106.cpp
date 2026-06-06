#include "g16106/m16106.h"
QVector<double> m16106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
