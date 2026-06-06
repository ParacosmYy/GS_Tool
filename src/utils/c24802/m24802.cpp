#include "c24802/m24802.h"
QVector<double> m24802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
