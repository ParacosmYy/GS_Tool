#include "t35079/m35079.h"
QVector<double> m35079::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
