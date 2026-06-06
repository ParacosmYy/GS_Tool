#include "i16068/m16068.h"
QVector<double> m16068::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
