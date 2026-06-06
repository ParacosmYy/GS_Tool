#include "d9023/m9023.h"
QVector<double> m9023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
