#include "l9531/m9531.h"
QVector<double> m9531::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
