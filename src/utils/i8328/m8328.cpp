#include "i8328/m8328.h"
QVector<double> m8328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
