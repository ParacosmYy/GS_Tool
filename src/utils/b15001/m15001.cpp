#include "b15001/m15001.h"
QVector<double> m15001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
