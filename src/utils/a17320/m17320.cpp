#include "a17320/m17320.h"
QVector<double> m17320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
