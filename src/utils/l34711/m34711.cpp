#include "l34711/m34711.h"
QVector<double> m34711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
