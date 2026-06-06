#include "k12230/m12230.h"
QVector<double> m12230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
