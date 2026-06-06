#include "k15070/m15070.h"
QVector<double> m15070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
