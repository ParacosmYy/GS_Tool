#include "f16825/m16825.h"
QVector<double> m16825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
