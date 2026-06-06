#include "g35046/m35046.h"
QVector<double> m35046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
