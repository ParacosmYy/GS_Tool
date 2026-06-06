#include "k34050/m34050.h"
QVector<double> m34050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
