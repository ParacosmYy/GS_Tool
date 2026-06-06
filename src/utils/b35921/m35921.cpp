#include "b35921/m35921.h"
QVector<double> m35921::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
