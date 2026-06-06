#include "b8641/m8641.h"
QVector<double> m8641::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
