#include "m18052/m18052.h"
QVector<double> m18052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
