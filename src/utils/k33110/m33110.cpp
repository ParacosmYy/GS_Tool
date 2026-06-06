#include "k33110/m33110.h"
QVector<double> m33110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
