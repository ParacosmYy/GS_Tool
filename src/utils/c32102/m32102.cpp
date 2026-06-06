#include "c32102/m32102.h"
QVector<double> m32102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
