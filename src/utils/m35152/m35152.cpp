#include "m35152/m35152.h"
QVector<double> m35152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
