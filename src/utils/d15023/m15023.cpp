#include "d15023/m15023.h"
QVector<double> m15023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
