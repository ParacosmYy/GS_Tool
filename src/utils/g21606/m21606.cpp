#include "g21606/m21606.h"
QVector<double> m21606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
