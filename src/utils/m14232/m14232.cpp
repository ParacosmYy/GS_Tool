#include "m14232/m14232.h"
QVector<double> m14232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
