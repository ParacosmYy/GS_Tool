#include "p35115/m35115.h"
QVector<double> m35115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
