#include "a10600/m10600.h"
QVector<double> m10600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
