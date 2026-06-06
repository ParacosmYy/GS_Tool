#include "i8288/m8288.h"
QVector<double> m8288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
