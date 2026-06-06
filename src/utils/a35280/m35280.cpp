#include "a35280/m35280.h"
QVector<double> m35280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
