#include "s16718/m16718.h"
QVector<double> m16718::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
