#include "g9226/m9226.h"
QVector<double> m9226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
