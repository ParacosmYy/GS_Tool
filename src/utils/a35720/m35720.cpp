#include "a35720/m35720.h"
QVector<double> m35720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
