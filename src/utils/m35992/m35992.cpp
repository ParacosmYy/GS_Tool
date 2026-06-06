#include "m35992/m35992.h"
QVector<double> m35992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
