#include "g25226/m25226.h"
QVector<double> m25226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
