#include "g21206/m21206.h"
QVector<double> m21206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
