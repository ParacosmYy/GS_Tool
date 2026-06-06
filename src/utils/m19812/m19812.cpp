#include "m19812/m19812.h"
QVector<double> m19812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
