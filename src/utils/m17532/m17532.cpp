#include "m17532/m17532.h"
QVector<double> m17532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
