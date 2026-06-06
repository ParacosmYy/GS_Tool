#include "m16892/m16892.h"
QVector<double> m16892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
