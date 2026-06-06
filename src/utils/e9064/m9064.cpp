#include "e9064/m9064.h"
QVector<double> m9064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
