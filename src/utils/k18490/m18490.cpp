#include "k18490/m18490.h"
QVector<double> m18490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
