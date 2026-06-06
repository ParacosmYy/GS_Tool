#include "k12870/m12870.h"
QVector<double> m12870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
