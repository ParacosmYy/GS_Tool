#include "m35172/m35172.h"
QVector<double> m35172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
