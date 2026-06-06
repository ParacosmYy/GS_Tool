#include "m32972/m32972.h"
QVector<double> m32972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
