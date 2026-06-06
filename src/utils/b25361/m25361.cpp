#include "b25361/m25361.h"
QVector<double> m25361::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
