#include "a16980/m16980.h"
QVector<double> m16980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
