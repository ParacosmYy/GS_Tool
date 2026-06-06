#include "b25001/m25001.h"
QVector<double> m25001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
