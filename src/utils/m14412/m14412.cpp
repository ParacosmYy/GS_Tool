#include "m14412/m14412.h"
QVector<double> m14412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
