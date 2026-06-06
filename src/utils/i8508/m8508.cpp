#include "i8508/m8508.h"
QVector<double> m8508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
