#include "m34512/m34512.h"
QVector<double> m34512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
