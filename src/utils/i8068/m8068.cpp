#include "i8068/m8068.h"
QVector<double> m8068::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
