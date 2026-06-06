#include "m8772/m8772.h"
QVector<double> m8772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
