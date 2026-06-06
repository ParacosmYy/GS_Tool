#include "h25087/m25087.h"
QVector<double> m25087::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
