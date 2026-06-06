#include "a35120/m35120.h"
QVector<double> m35120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
