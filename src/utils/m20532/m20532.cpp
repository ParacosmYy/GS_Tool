#include "m20532/m20532.h"
QVector<double> m20532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
