#include "q8036/m8036.h"
QVector<double> m8036::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
