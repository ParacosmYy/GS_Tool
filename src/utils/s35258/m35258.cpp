#include "s35258/m35258.h"
QVector<double> m35258::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
