#include "b25541/m25541.h"
QVector<double> m25541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
