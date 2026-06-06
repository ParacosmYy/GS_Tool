#include "c8382/m8382.h"
QVector<double> m8382::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
