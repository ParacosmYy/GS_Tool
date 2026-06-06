#include "a10620/m10620.h"
QVector<double> m10620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
