#include "m16972/m16972.h"
QVector<double> m16972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
