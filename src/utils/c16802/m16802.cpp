#include "c16802/m16802.h"
QVector<double> m16802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
