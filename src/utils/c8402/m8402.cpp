#include "c8402/m8402.h"
QVector<double> m8402::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
