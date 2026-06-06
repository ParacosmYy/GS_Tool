#include "a35520/m35520.h"
QVector<double> m35520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
