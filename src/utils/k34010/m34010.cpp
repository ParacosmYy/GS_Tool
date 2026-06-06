#include "k34010/m34010.h"
QVector<double> m34010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
