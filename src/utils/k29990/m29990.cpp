#include "k29990/m29990.h"
QVector<double> m29990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
