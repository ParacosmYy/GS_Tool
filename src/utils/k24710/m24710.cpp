#include "k24710/m24710.h"
QVector<double> m24710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
