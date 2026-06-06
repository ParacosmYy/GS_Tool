#include "t32099/m32099.h"
QVector<double> m32099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
