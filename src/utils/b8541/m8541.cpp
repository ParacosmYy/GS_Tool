#include "b8541/m8541.h"
QVector<double> m8541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
