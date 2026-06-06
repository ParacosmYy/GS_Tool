#include "m17872/m17872.h"
QVector<double> m17872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
