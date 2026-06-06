#include "c9802/m9802.h"
QVector<double> m9802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
