#include "i16028/m16028.h"
QVector<double> m16028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
