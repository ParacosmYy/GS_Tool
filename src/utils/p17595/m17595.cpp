#include "p17595/m17595.h"
QVector<double> m17595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
