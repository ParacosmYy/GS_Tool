#include "c25802/m25802.h"
QVector<double> m25802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
