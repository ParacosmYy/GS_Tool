#include "m16172/m16172.h"
QVector<double> m16172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
