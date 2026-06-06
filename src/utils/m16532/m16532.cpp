#include "m16532/m16532.h"
QVector<double> m16532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
