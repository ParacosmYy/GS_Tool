#include "k35630/m35630.h"
QVector<double> m35630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
