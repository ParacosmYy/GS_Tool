#include "h15827/m15827.h"
QVector<double> m15827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
