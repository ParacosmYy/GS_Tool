#include "k12810/m12810.h"
QVector<double> m12810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
