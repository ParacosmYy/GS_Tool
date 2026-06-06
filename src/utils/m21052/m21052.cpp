#include "m21052/m21052.h"
QVector<double> m21052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
