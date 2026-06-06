#include "k12910/m12910.h"
QVector<double> m12910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
