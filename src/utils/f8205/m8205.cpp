#include "f8205/m8205.h"
QVector<double> m8205::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
