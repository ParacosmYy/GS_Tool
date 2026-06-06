#include "c16202/m16202.h"
QVector<double> m16202::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
