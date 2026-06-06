#include "r35357/m35357.h"
QVector<double> m35357::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
