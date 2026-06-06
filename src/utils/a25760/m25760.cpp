#include "a25760/m25760.h"
QVector<double> m25760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
