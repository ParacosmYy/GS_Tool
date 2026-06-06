#include "f35705/m35705.h"
QVector<double> m35705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
