#include "c24402/m24402.h"
QVector<double> m24402::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
