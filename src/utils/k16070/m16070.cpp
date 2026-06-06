#include "k16070/m16070.h"
QVector<double> m16070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
