#include "m25812/m25812.h"
QVector<double> m25812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
