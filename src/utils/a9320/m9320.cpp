#include "a9320/m9320.h"
QVector<double> m9320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
