#include "h18887/m18887.h"
QVector<double> m18887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
