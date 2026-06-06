#include "k12390/m12390.h"
QVector<double> m12390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
