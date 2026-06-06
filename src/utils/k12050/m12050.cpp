#include "k12050/m12050.h"
QVector<double> m12050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
