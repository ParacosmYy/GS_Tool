#include "f8405/m8405.h"
QVector<double> m8405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
