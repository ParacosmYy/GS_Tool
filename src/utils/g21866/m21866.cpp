#include "g21866/m21866.h"
QVector<double> m21866::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
