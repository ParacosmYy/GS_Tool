#include "a24720/m24720.h"
QVector<double> m24720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
