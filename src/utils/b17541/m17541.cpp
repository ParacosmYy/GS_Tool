#include "b17541/m17541.h"
QVector<double> m17541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
