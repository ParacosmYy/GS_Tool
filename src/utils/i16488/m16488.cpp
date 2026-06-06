#include "i16488/m16488.h"
QVector<double> m16488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
