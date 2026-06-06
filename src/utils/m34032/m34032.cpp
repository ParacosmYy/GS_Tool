#include "m34032/m34032.h"
QVector<double> m34032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
