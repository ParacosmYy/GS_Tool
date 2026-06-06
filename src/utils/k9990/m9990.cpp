#include "k9990/m9990.h"
QVector<double> m9990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
