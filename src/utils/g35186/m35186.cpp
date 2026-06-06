#include "g35186/m35186.h"
QVector<double> m35186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
