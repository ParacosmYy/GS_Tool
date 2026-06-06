#include "a16320/m16320.h"
QVector<double> m16320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
