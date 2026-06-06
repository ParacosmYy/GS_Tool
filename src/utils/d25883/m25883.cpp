#include "d25883/m25883.h"
QVector<double> m25883::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
