#include "g18866/m18866.h"
QVector<double> m18866::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
