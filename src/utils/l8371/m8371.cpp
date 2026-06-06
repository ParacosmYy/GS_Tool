#include "l8371/m8371.h"
QVector<double> m8371::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
