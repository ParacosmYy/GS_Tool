#include "m35192/m35192.h"
QVector<double> m35192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
