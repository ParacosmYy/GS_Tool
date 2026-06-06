#include "k34110/m34110.h"
QVector<double> m34110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
