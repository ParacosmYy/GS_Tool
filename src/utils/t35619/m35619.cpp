#include "t35619/m35619.h"
QVector<double> m35619::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
