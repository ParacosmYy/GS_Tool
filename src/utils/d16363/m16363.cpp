#include "d16363/m16363.h"
QVector<double> m16363::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
