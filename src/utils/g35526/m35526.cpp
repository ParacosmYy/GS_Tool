#include "g35526/m35526.h"
QVector<double> m35526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
