#include "m9812/m9812.h"
QVector<double> m9812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
