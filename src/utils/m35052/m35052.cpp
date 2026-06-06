#include "m35052/m35052.h"
QVector<double> m35052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
