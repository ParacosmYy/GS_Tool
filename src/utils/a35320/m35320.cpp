#include "a35320/m35320.h"
QVector<double> m35320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
