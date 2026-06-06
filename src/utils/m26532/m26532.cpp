#include "m26532/m26532.h"
QVector<double> m26532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
