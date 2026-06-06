#include "m14552/m14552.h"
QVector<double> m14552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
