#include "t8219/m8219.h"
QVector<double> m8219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
