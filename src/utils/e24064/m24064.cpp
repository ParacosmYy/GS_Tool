#include "e24064/m24064.h"
QVector<double> m24064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
