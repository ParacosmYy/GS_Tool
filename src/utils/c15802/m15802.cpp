#include "c15802/m15802.h"
QVector<double> m15802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
