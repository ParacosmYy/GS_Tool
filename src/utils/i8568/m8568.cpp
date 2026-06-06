#include "i8568/m8568.h"
QVector<double> m8568::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
