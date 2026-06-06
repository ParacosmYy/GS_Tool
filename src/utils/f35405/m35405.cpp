#include "f35405/m35405.h"
QVector<double> m35405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
