#include "a32340/m32340.h"
QVector<double> m32340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
