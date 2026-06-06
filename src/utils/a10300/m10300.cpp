#include "a10300/m10300.h"
QVector<double> m10300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
