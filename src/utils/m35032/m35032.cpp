#include "m35032/m35032.h"
QVector<double> m35032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
