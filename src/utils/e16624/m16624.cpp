#include "e16624/m16624.h"
QVector<double> m16624::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
