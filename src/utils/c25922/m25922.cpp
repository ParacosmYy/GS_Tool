#include "c25922/m25922.h"
QVector<double> m25922::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
