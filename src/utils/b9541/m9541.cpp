#include "b9541/m9541.h"
QVector<double> m9541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
