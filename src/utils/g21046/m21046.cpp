#include "g21046/m21046.h"
QVector<double> m21046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
