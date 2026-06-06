#include "m17552/m17552.h"
QVector<double> m17552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
