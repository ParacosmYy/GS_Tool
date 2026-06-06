#include "a35060/m35060.h"
QVector<double> m35060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
