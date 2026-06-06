#include "k29710/m29710.h"
QVector<double> m29710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
