#include "c32802/m32802.h"
QVector<double> m32802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
