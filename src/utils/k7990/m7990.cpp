#include "k7990/m7990.h"
QVector<double> m7990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
