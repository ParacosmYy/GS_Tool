#include "d35323/m35323.h"
QVector<double> m35323::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
