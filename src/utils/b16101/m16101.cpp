#include "b16101/m16101.h"
QVector<double> m16101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
