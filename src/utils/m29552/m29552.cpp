#include "m29552/m29552.h"
QVector<double> m29552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
