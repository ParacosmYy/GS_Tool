#include "l8531/m8531.h"
QVector<double> m8531::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
