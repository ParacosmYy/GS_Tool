#include "d9383/m9383.h"
QVector<double> m9383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
