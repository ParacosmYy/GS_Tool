#include "m21132/m21132.h"
QVector<double> m21132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
