#include "a28060/m28060.h"
QVector<double> m28060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
