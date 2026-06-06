#include "m36552/m36552.h"
QVector<double> m36552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
