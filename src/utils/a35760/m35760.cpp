#include "a35760/m35760.h"
QVector<double> m35760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
