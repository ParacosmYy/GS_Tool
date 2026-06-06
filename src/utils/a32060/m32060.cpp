#include "a32060/m32060.h"
QVector<double> m32060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
