#include "n36713/m36713.h"
QVector<double> m36713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
