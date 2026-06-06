#include "n8713/m8713.h"
QVector<double> m8713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
