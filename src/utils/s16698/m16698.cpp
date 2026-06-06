#include "s16698/m16698.h"
QVector<double> m16698::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
