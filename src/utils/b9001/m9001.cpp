#include "b9001/m9001.h"
QVector<double> m9001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
