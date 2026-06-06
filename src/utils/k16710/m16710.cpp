#include "k16710/m16710.h"
QVector<double> m16710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
