#include "l9111/m9111.h"
QVector<double> m9111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
