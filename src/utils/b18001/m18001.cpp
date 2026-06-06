#include "b18001/m18001.h"
QVector<double> m18001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
