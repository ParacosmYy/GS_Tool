#include "t35199/m35199.h"
QVector<double> m35199::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
