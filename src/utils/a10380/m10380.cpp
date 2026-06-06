#include "a10380/m10380.h"
QVector<double> m10380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
