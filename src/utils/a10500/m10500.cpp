#include "a10500/m10500.h"
QVector<double> m10500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
