#include "g35226/m35226.h"
QVector<double> m35226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
