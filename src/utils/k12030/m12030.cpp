#include "k12030/m12030.h"
QVector<double> m12030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
