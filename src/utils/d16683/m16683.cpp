#include "d16683/m16683.h"
QVector<double> m16683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
