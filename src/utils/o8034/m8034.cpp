#include "o8034/m8034.h"
QVector<double> m8034::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
