#include "i8968/m8968.h"
QVector<double> m8968::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
