#include "t37099/m37099.h"
QVector<double> m37099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
