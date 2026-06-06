#include "i8108/m8108.h"
QVector<double> m8108::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
