#include "n32713/m32713.h"
QVector<double> m32713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
