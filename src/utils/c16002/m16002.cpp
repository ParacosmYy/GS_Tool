#include "c16002/m16002.h"
QVector<double> m16002::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
