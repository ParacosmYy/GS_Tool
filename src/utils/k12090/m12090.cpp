#include "k12090/m12090.h"
QVector<double> m12090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
