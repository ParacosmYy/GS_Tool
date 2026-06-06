#include "b8041/m8041.h"
QVector<double> m8041::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
