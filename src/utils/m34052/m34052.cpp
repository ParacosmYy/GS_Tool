#include "m34052/m34052.h"
QVector<double> m34052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
