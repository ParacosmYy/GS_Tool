#include "b24541/m24541.h"
QVector<double> m24541::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
