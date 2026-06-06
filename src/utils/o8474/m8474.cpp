#include "o8474/m8474.h"
QVector<double> m8474::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
