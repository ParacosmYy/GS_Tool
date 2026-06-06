#include "o35014/m35014.h"
QVector<double> m35014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
