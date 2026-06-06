#include "t28099/m28099.h"
QVector<double> m28099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
