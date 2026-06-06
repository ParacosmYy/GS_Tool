#include "m21192/m21192.h"
QVector<double> m21192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
