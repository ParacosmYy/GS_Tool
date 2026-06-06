#include "s25518/m25518.h"
QVector<double> m25518::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
