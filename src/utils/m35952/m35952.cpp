#include "m35952/m35952.h"
QVector<double> m35952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
