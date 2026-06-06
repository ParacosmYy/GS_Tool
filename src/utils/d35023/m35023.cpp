#include "d35023/m35023.h"
QVector<double> m35023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
