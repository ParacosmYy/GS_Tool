#include "i16688/m16688.h"
QVector<double> m16688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
