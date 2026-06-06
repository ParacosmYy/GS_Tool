#include "t24099/m24099.h"
QVector<double> m24099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
