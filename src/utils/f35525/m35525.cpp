#include "f35525/m35525.h"
QVector<double> m35525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
