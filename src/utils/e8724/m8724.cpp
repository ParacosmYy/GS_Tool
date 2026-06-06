#include "e8724/m8724.h"
QVector<double> m8724::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
