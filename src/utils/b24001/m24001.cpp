#include "b24001/m24001.h"
QVector<double> m24001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
