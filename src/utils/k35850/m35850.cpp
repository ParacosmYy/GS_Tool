#include "k35850/m35850.h"
QVector<double> m35850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
